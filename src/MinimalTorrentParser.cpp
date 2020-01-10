#include <functional>
#include <openssl/sha.h>

#include "MinimalTorrentParser.hpp"
#include "Logger.hpp"
#include "Defaults.hpp"
#include "Config.hpp"
#include "helpers.hpp"

namespace {
	static char constexpr BC_START_INT = 'i';
	// static char constexpr BC_START_STRING = 's';
	static char constexpr BC_END_STRLEN = ':';
    static char constexpr BC_START_DICT = 'd';
    static char constexpr BC_START_LIST = 'l';
    static char constexpr BC_END_VALUE = 'e';

    std::string getSHA1Hash(/* IN */ char const * const buffer) {
        char hashValue[BT::Defaults::Sha1MdSize] = "";
        SHA1((unsigned char *)buffer, std::string(buffer).length(), (unsigned char *)hashValue);
        hashValue[BT::Defaults::Sha1MdSize - 1] = '\0';
        return std::string(hashValue);
    }
}

namespace BT {
	MinimalTorrentParser_t::MinimalTorrentParser_t()
	            : mInfoDict(false),
	              mStartInfoDict(0),
	              mEndInfoDict(0) {}

	MinimalTorrentParser_t::~MinimalTorrentParser_t() {
	    if(mIfstream.is_open())
	        mIfstream.close();
	}

	STATUSCODE  MinimalTorrentParser_t::Parse(/* IN */  std::string const& fileName,
	                                          /* OUT */ Metainfo_t& rInfo) const {
		rInfo.reset();

		mIfstream.open(fileName.c_str());
	    if (!mIfstream.is_open()) {
	        Error("Unable to open .torrent file: %s", fileName.c_str());
	        return STATUSCODE::SC_FAIL_UNKNOWN;
	    }

	    char ch = '\0';
	    STATUSCODE status = extractChar(ch);
	    if (SC_FAILED(status) || ch != BC_START_DICT) {
	        Error("Torrent file is not in Bencode format.");
	        return STATUSCODE::SC_FAIL_BAD_TORRENT;
	    }

	    MI_Object_t bcDict = extract_MI_Dict();
		rInfo.mData = *(bcDict.value.mDict.get()); 

	    char buffer[BT::Defaults::MaxBufferSize] = "";
	    mIfstream.seekg(mStartInfoDict);
	    mIfstream.read(buffer, mEndInfoDict - mStartInfoDict + 1);
	    rInfo.mInfoHash = getSHA1Hash(buffer);

	    return STATUSCODE::SC_SUCCESS;
	}

	STATUSCODE MinimalTorrentParser_t::extractChar(/* OUT */ char& rCh) const {
	    rCh = '\0';

	    if (mIfstream.eof()) {
	        Error("EOF while reading character from the torrent file.");
	        return STATUSCODE::SC_FAIL_BAD_TORRENT;
	    }
	    
	    rCh = mIfstream.get();
	    return STATUSCODE::SC_SUCCESS;
	}

	STATUSCODE MinimalTorrentParser_t::extractLong(/* IN */  char delim,
	                                               /* OUT */ long& rOut) const {
	    rOut = 0;

	    if (mIfstream.eof()) {
	        Error("EOF while reading integer from the torrent file.");
	        return STATUSCODE::SC_FAIL_BAD_TORRENT;
	    }

	    char buffer[BT::Defaults::MaxBufferSize] = "";
	    mIfstream.get(buffer, BT::Defaults::MaxBufferSize, delim);
	    rOut = std::stol(buffer);
	    return STATUSCODE::SC_SUCCESS;
	}

	BT::MI_Object_t MinimalTorrentParser_t::extract_MI_Int() const {
	    long value = 0;
	    STATUSCODE status = extractLong(BC_END_VALUE, value);
	    if (SC_FAILED(status)) {
	        Error("Torrent parsing failed while extracting MI_INT.");
	        value = 0;
	    }

	    MI_Object_t obj;
	    obj.kind = BT::MI_ObjectKind_t::MI_INT;
	    obj.value.nInt = static_cast<BT::MI_Int_t>(value);
	    return obj;
	}

	BT::MI_Object_t MinimalTorrentParser_t::extract_MI_String() const {
	    long strLen = 0;
	    char buffer[BT::Defaults::MaxBufferSize] = "";
	    
	    STATUSCODE status = extractLong(BC_END_STRLEN, strLen);
	    if (SC_FAILED(status)) {
	        Error("Torrent parsing failed while extracting MI_STRING");
	        strLen = 0;
	    }
	    else if (mIfstream.eof()) {
	        Error("EOF while parsing string from the torrent file.");
	        buffer[0] = '\0';
	    }
	    else
	        mIfstream.get(buffer, strLen);

	    /* Special processing to remove the '\0' char within the message*/
	    for (int i = 0; i < strLen; i++)
	        if ('\0' == buffer[i])
	            buffer[i] = '_';

	    BT::MI_Object_t obj;
	    obj.kind = BT::MI_ObjectKind_t::MI_STRING;
	    obj.value.cStr = static_cast<BT::MI_String_t>(std::string(buffer));

	    return obj;
	}

	BT::MI_Object_t MinimalTorrentParser_t::extract_MI_List() const {
	    auto data = std::make_shared<BT::MI_List_t>(MI_List_t());

	    while (true) {
	        char ch = '\0';
	        STATUSCODE status = extractChar(ch);
	        if (SC_FAILED(status)) {
	            Error("Torrent parsing failed when extracting MI_LIST.");
	            data->clear();
	            break;
	        }

	        if (ch == BC_END_VALUE)
	            break;

	        BT::MI_Object_t value = extractData(ch);
	        data->push_back(value);
	    }

	    BT::MI_Object_t obj;
	    obj.kind = BT::MI_ObjectKind_t::MI_LIST;
	    obj.value.vList = data;

	    return obj;
	}

	BT::MI_Object_t MinimalTorrentParser_t::extract_MI_Dict() const {
	    auto data = std::make_shared<BT::MI_Dict_t>(MI_Dict_t());

	    auto endOfDict = [&]() {
	        char ch = '\0';
	        STATUSCODE status = extractChar(ch);
	        if (SC_FAILED(status)) {
	            Error("Torrent parsing failed when extracting MI_DICT.");
	            return true;
	        }

	        if (ch == BC_END_VALUE)
	            return true;

	        mIfstream.seekg(-1, std::ios::cur);
	        return false;
	    };

	    while (true) {
	        BT::MI_String_t key = extract_MI_String().value.cStr;

	        if (0 == key.compare("info")) {
	            mInfoDict = true;
	            mStartInfoDict = mIfstream.tellg();
	        }

	        char ch = '\0';
	        STATUSCODE status = extractChar(ch);
	        if (SC_FAILED(status)) {
	            Error("Torrent parsing failed when extracting MI_DICT.");
	        } else {
	            BT::MI_Object_t value = extractData(ch);
	            data->emplace(key, value);
	        }

	        if (endOfDict()) {
	            if (mInfoDict) {
	                mEndInfoDict = mIfstream.tellg();
	                mInfoDict = false;
	            }
	            break;
	        }
	    }

	    BT::MI_Object_t obj;
	    obj.kind = BT::MI_ObjectKind_t::MI_DICT;
	    obj.value.mDict = data;

	    return obj;
	}

	BT::MI_Object_t MinimalTorrentParser_t::extractData(char ch) const {
	    static std::map<char, std::function<BT::MI_Object_t(void)>> handlerMap;
	    if (handlerMap.empty()) {
	        handlerMap[BC_START_INT] = [&]() { return extract_MI_Int(); };
	        handlerMap[BC_START_LIST] = [&]() { return extract_MI_List(); };
	        handlerMap[BC_START_DICT] = [&]() { return extract_MI_Dict(); };
	    }
	    
	    auto handlerItr = handlerMap.find(ch);
	    if (handlerItr != handlerMap.end())
	        return handlerItr->second();

	    mIfstream.seekg(-1, std::ios::cur);
	    return extract_MI_String();
	}
}