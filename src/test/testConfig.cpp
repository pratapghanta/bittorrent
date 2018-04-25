#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#include "Config.hpp"
#include "ConfigExceptions.hpp"

namespace {
	std::vector<std::string> getVectorOfString(std::string const& data) {
		std::vector<std::string> dataVec;
		if (data.empty())
			return dataVec;

		std::istringstream iss(data);

		std::string token;
		while (std::getline(iss, token, ' '))
			if (!token.empty())
				dataVec.push_back(token);

		return dataVec;
	}
}

TEST(testArgumentParsing, noArguments) {
	std::string const args("");
	EXPECT_ANY_THROW(BT::Config_t(getVectorOfString(args)));
}

TEST(testArgumentParsing, unknownArgument) {
	std::string const args("-a file.torrent");
 	EXPECT_ANY_THROW(BT::Config_t(getVectorOfString(args)));
}

TEST(testArgumentParsing, extraArgument) {
	std::string const args("file1.torrent file1.torrent");
	EXPECT_ANY_THROW(BT::Config_t(getVectorOfString(args)));
}

TEST(testArgumentParsing, missingArgument) {
	std::string const args("-b file1.torrent");
	EXPECT_ANY_THROW(BT::Config_t(getVectorOfString(args)));
}

TEST(testArgumentParsing, helpNotRequested) {
	std::string const args("file.torrent");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_FALSE(config.isHelpRequested());
}

TEST(testArgumentParsing, helpWithoutAnyOtherArguments) {
	std::string const args("-h");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_TRUE(config.isHelpRequested());
}

TEST(testArgumentParsing, helpWithOtherArguments_start) {
	std::string const args("-h -b 3000 -s /saveFile.txt -l /bittorrent.logs -p 127.0.0.1:6167 -v");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_TRUE(config.isHelpRequested());
}

TEST(testArgumentParsing, helpWithOtherArguments_middle) {
	std::string const args("-b 3000 -s /saveFile.txt -l /bittorrent.logs -h -p 127.0.0.1:6167 -v /file.torrent");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_TRUE(config.isHelpRequested());
}

TEST(testArgumentParsing, helpWithOtherArguments_end) {
	std::string const args("-b 3000 -s /saveFile.txt -l /bittorrent.logs -p 127.0.0.1:6167 -v /file.torrent -h");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_TRUE(config.isHelpRequested());
}

TEST(testArgumentParsing, verboseDisabled) {
	std::string const args("file.torrent");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_FALSE(config.isVerbose());
}

TEST(testArgumentParsing, verboseEnabled) {
	std::string const args("-v /file.torrent");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_TRUE(config.isVerbose());
}

TEST(testArgumentParsing, seederPort) {
	std::string const args("-b 3000 -s /saveFile.txt -l /bittorrent.logs -p 127.0.0.1:6167 -v /file.torrent -h");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_EQ(config.getSeederPort(), 3000u);
}

TEST(testArgumentParsing, inValidSeederPort) {
	std::string const args("-b 80");
	EXPECT_ANY_THROW(BT::Config_t(getVectorOfString(args)));
}

TEST(testArgumentParsing, saveFileName) {
	std::string const args("-b 3000 -s /saveFile.txt -l /bittorrent.logs -p 127.0.0.1:6167 -v /file.torrent -h");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_TRUE(config.getSaveFilename().compare(std::string("/saveFile.txt")) == 0);
}

TEST(testArgumentParsing, logFileName) {
	std::string const args("-b 3000 -s /saveFile.txt -l /bittorrent.logs -p 127.0.0.1:6167 -v /file.torrent -h");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_TRUE(config.getLogFilename().compare(std::string("/bittorrent.logs")) == 0);
}

TEST(testArgumentParsing, enableVerbose) {
	std::string const args("-b 3000 -s /saveFile.txt -l /bittorrent.logs -p 127.0.0.1:6167 -v /file.torrent -h");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_TRUE(config.isVerbose());
}

TEST(testArgumentParsing, peerInformation) {
	std::string const args("-l /bittorrent.logs -p 127.0.0.1:6167 -p 127.0.0.1:6168 -p 127.0.0.1:6169 -v /file.torrent -h");
	auto const& config = BT::Config_t(getVectorOfString(args));
	auto const& peersInfo = config.getPeers();

	auto verify = [&](unsigned int const i) { return (peersInfo[i].getIp().compare("127.0.0.1") == 0) && (peersInfo[i].getPort() == 6167u+i); };
	for (unsigned int i = 0; i < 3; i++)
		EXPECT_TRUE(verify(i));
}

TEST(testArgumentParsing, invalidPeerIP) {
	std::string const args("-b 3000 -s /saveFile.txt -l /bittorrent.logs -p 256.0.0.1:6167 -v /file.torrent");
	EXPECT_ANY_THROW(BT::Config_t(getVectorOfString(args)).getPeers());
}

TEST(testArgumentParsing, invalidPeerPort) {
	std::string const args("-b 3000 -s /saveFile.txt -l /bittorrent.logs -p 256.0.0.1:43 -v /file.torrent");
	EXPECT_ANY_THROW(BT::Config_t(getVectorOfString(args)).getPeers());
}

TEST(testArgumentParsing, identifyLeecher) {
	std::string const args("-l /bittorrent.logs -p 127.0.0.1:6167 -p 127.0.0.1:6168 -p 127.0.0.1:6169 -v /file.torrent -h");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_FALSE(config.isSeeder());
}

TEST(testArgumentParsing, identifySeeder) {
	std::string const args("-l /bittorrent.logs -v /file.torrent -h");
	auto const& config = BT::Config_t(getVectorOfString(args));
	EXPECT_TRUE(config.isSeeder());
}

TEST(testArgumentParsing, noExceptionWhenHelpIsRequested) {
	std::string const args("-b 100 -s /saveFile.txt -l /bittorrent.logs -p 256.0.0.1:43 -v -h");
	EXPECT_NO_THROW(BT::Config_t(getVectorOfString(args)));
}
