#include <functional>
#include <openssl/sha.h>

#include "common/Defaults.hpp"
#include "common/Helpers.hpp"
#include "common/Logger.hpp"
#include "torrent/MinimalTorrentParser.hpp"
#include "StartParams.hpp"

/* TODO: Add exception safety */

namespace {
    static char constexpr BC_START_INT = 'i';
    static char constexpr BC_END_STRLEN = ':';
    static char constexpr BC_START_DICT = 'd';
    static char constexpr BC_START_LIST = 'l';
    static char constexpr BC_END_VALUE = 'e';


    std::string getSHA1Hash(/* IN */ char const * const buffer) {
        char hashValue[BT::Defaults::Sha1MdSize+1] = "";
        SHA1((unsigned char *)buffer, std::string(buffer).length(), (unsigned char *)hashValue);
        hashValue[BT::Defaults::Sha1MdSize] = '\0';
        return std::string(hashValue);
    }
}


namespace BT {
    MinimalTorrentParser_t::MinimalTorrentParser_t()
        : mInfoDict(false),
          mStartInfoDict(0),
          mEndInfoDict(0) {}


    MinimalTorrentParser_t::MinimalTorrentParser_t(/* IN */ MinimalTorrentParser_t&& mtp)
        : mInfoDict(false),
          mStartInfoDict(0),
          mEndInfoDict(0) {
        *this = std::move(mtp);
    }


    MinimalTorrentParser_t& MinimalTorrentParser_t::operator=(/* IN */ MinimalTorrentParser_t&& mtp) {
        if (this != &mtp) {
            if (mIfstream.is_open())
                mIfstream.close();            
            mIfstream = std::move(mtp.mIfstream);
            mInfoDict = mtp.mInfoDict;
            mStartInfoDict = mtp.mStartInfoDict;
            mEndInfoDict = mtp.mEndInfoDict;

            mtp.reset();
        }
        
        return mtp;
    }


    void MinimalTorrentParser_t::reset() {
        if (mIfstream.is_open())
            mIfstream.close();
        mInfoDict = false;
        mStartInfoDict = 0;
        mEndInfoDict = 0;
    }


    STATUSCODE  MinimalTorrentParser_t::Parse(/* IN  */ std::string const& strFileName,
                                              /* OUT */ Metainfo_t& rInfo) const {
        rInfo.Reset();

        STATUSCODE status = openFile(strFileName);
        if (SC_FAILED(status))
            return status;

        char ch = '\0';
        status = extractChar(ch);
        if (SC_FAILED(status) || ch != BC_START_DICT) {
            Error("File is not in Bencode format.");
            return STATUSCODE::SC_FAIL_BAD_TORRENT;
        }

        MI_Object_t bcDict = extract_MI_Dict();
        rInfo.mData = *(bcDict.GetDictPtr()); 
        return computeInfoDictHash(rInfo.mInfoHash);
    }


    STATUSCODE MinimalTorrentParser_t::openFile(/* IN */ std::string const& strFileName) const {
        if (mIfstream.is_open())
            mIfstream.close();

        mIfstream.open(strFileName.c_str());
        if (!mIfstream.is_open()) {
            Error("Unable to open file: %s", strFileName.c_str());
            return STATUSCODE::SC_FAIL_UNKNOWN;
        }

        return STATUSCODE::SC_SUCCESS;
    }


    STATUSCODE MinimalTorrentParser_t::computeInfoDictHash(/* OUT */ std::string& rHash) const {
        rHash.clear();

        if (!mIfstream.is_open() || 
            mEndInfoDict < mStartInfoDict || 
            mEndInfoDict - mStartInfoDict + 1 >= BT::Defaults::MaxBufferSize ) {
            Error("Unable to compute hash value for Info dictionary.");
            return STATUSCODE::SC_FAIL_UNKNOWN;
        }

        char buffer[BT::Defaults::MaxBufferSize] = "";
        mIfstream.seekg(mStartInfoDict);
        mIfstream.read(buffer, mEndInfoDict - mStartInfoDict + 1);
        rHash = getSHA1Hash(buffer);

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


    STATUSCODE MinimalTorrentParser_t::extractLong(/* IN  */ char const delim,
                                                   /* OUT */ long& rOut) const {
        rOut = 0;

        if (mIfstream.eof()) {
            Error("EOF while reading integer from the torrent file.");
            return STATUSCODE::SC_FAIL_BAD_TORRENT;
        }

        char buffer[BT::Defaults::MaxBufferSize] = "";
        mIfstream.get(buffer, BT::Defaults::MaxBufferSize, delim);
        rOut = std::stol(buffer);

        char ch = '\0';
        STATUSCODE status = extractChar(ch);
        if (SC_FAILED(status) || ch != delim) {
            Error("EOF while reading integer from the torrent file.");
            return STATUSCODE::SC_FAIL_BAD_TORRENT;
        }

        return STATUSCODE::SC_SUCCESS;
    }

    
    /* Format: i<contents>e */
    BT::MI_Object_t MinimalTorrentParser_t::extract_MI_Int() const {
        long value = 0;

        STATUSCODE status = extractLong(BC_END_VALUE, value);
        if (SC_FAILED(status)) {
            Error("Torrent parsing failed while extracting MI_INT.");
            value = 0;
        }

        return MI_Object_t(static_cast<MI_Int_t>(value));
    }


    /* Format: length:string */
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
            mIfstream.get(buffer, strLen+1);

        /* Special processing to remove the '\0' char within the string */
        for (int i = 0; i < strLen; i++)
            if ('\0' == buffer[i])
                buffer[i] = '_';
        buffer[strLen] = '\0';

        return BT::MI_Object_t(static_cast<MI_String_t>(std::string(buffer)));
    }

    /* Format: l<contents>e */
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

            data->push_back(extractData(ch));
        }

        return BT::MI_Object_t(data);
    }

    /* Format: d<contents>e */
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
            BT::MI_Object_t keyObj = extract_MI_String();
            BT::MI_String_t key = keyObj.GetString();

            if (0 == key.compare("info")) {
                mInfoDict = true;
                mStartInfoDict = mIfstream.tellg();
            }

            char ch = '\0';
            STATUSCODE status = extractChar(ch);
            if (SC_FAILED(status)) {
                Error("Torrent parsing failed when extracting MI_DICT.");
            } else {
                data->emplace(key, extractData(ch));
            }

            if (endOfDict()) {
                if (mInfoDict) {
                    mEndInfoDict = mIfstream.tellg();
                    mInfoDict = false;
                }
                break;
            }
        }

        return BT::MI_Object_t(data);
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
