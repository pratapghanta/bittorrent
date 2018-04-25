#include <iostream>
#include <fstream>
#include <functional>
#include <openssl/sha.h>

#include "Defaults.hpp"
#include "Config.hpp"
#include "helpers.hpp"
#include "MinimalTorrentParser.hpp"

namespace {
	char const extractChar(std::ifstream& ifs) {
		if (ifs.eof())
			throwErrorAndExit("Invalid .torrent file format.");
		
		return ifs.get();
	}

	long const extractLong(std::ifstream& ifs, char delim) {
		char buffer[BT::Defaults::MaxBufferSize] = "";
		ifs.get(buffer, BT::Defaults::MaxBufferSize, delim);
		return std::stol(buffer);
	}

	std::string const getSHA1Hash(char const * const buffer) {
		char hashValue[BT::Defaults::Sha1MdSize] = "";
		SHA1((unsigned char *)buffer, std::string(buffer).length(), (unsigned char *)hashValue);
		hashValue[BT::Defaults::Sha1MdSize - 1] = '\0';
		return std::string(hashValue);
	}

	class BencodeParser {
	public:
		BencodeParser(std::string const& fileName) : ifs(fileName.c_str()), infoDict(false), startInfoDict(0), endInfoDict(0) {
			if (!ifs.is_open())
				throwErrorAndExit("Unable to open .torrent file.");
		}

		BT::Metainfo getBencodeDictInfo() {
			if (extractChar(ifs) != 'd')
				throwErrorAndExit("Invalid .torrent file format.");

			BT::MI_Object_t const bcDict = extractDict();
		
			char buffer[BT::Defaults::MaxBufferSize] = "";
			ifs.seekg(startInfoDict);
			ifs.read(buffer, endInfoDict - startInfoDict + 1);
			std::string const hashValue = getSHA1Hash(buffer);
			
			return BT::Metainfo(bcDict.value.mDict, hashValue);
		}

		~BencodeParser() {
			if(ifs.is_open())
				ifs.close();
		}

	private:
		BT::MI_Object_t extractLong() {
			BT::MI_Object_t obj;

			obj.kind = BT::MI_ObjectKind_t::MI_INT;
			obj.value.nInt = static_cast<BT::MI_Int_t>(::extractLong(ifs, 'e'));

			return obj;
		}

		BT::MI_Object_t extractString() {
			BT::MI_Object_t obj;

			unsigned int const strLen = ::extractLong(ifs, ':');
			char buffer[BT::Defaults::MaxBufferSize] = "";
			ifs.get(buffer, strLen);

			/* Special processing to remove the '\0' char within the message*/
			while (std::string(buffer).length() < strLen)
				buffer[std::string(buffer).length()] = '_';

			obj.kind = BT::MI_ObjectKind_t::MI_STRING;
			obj.value.cStr = static_cast<BT::MI_String_t>(std::string(buffer));

			return obj;
		}

		BT::MI_Object_t extractList() {
			auto data = std::make_shared<BT::MI_List_t>(BT::MI_List_t());

			while (true) {
				char ch = extractChar(ifs);
				if (ch == 'e')
					break;

				BT::MI_Object_t value = extractData(ch);
				data->push_back(value);
			}

			BT::MI_Object_t obj;
			obj.kind = BT::MI_ObjectKind_t::MI_LIST;
			obj.value.vList = data;

			return obj;
		}

		BT::MI_Object_t extractDict() {
			auto data = std::make_shared<BT::MI_Dict_t>(BT::MI_Dict_t());

			auto endOfDict = [](std::ifstream& ifs) {
				char ch = extractChar(ifs);
				if (ch == 'e')
					return true;

				ifs.seekg(-1, std::ios::cur);
				return false;
			};

			while (true) {
				BT::MI_String_t key = extractString().value.cStr;

				if (key.compare("info") == 0) {
					infoDict = true;
					startInfoDict = ifs.tellg();
				}

				char ch = extractChar(ifs);
				BT::MI_Object_t value = extractData(ch);

				data->emplace(key, value);

				if (endOfDict(ifs)) {
					if (infoDict) {
						endInfoDict = ifs.tellg();
						infoDict = false;
					}

					break;
				}
			}

			BT::MI_Object_t obj;
			obj.kind = BT::MI_ObjectKind_t::MI_DICT;
			obj.value.mDict = data;

			return obj;
		}

		BT::MI_Object_t extractData(char ch)
		{
			std::map<char, std::function<BT::MI_Object_t(void)>> handlerMap;
			handlerMap['i'] = [&]() { return extractLong(); };
			handlerMap['l'] = [&]() { return extractList(); };
			handlerMap['d'] = [&]() { return extractDict(); };

			auto handlerItr = handlerMap.find(ch);
			if (handlerItr != handlerMap.end())
				return handlerItr->second();

			ifs.seekg(-1, std::ios::cur);
			return extractString();
		}

		std::ifstream ifs;
		bool infoDict;
		std::streampos startInfoDict;
		std::streampos endInfoDict;
	};
}

namespace BT {
	Metainfo const MinimalTorrentParser::doParse(std::string const& fileName) const {
		BencodeParser bcp(fileName);
		return bcp.getBencodeDictInfo();
	}
}
