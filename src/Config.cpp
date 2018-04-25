#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>

#include <cstdlib>
#include <unistd.h>
#include <openssl/sha.h>

#include "Defaults.hpp"

#include "Config.hpp"
#include "ConfigExceptions.hpp"

namespace {
	bool const isValidIP(std::string const& str) {
		std::istringstream iss(str);
		char const delim = '.';
		std::string token;
		int nTokens = 0;

		while (std::getline(iss, token, delim)) {
			nTokens++;

			int value = std::stoi(token);
			if (!(value >= 0 && value <= 255))
				return false;
		}

		return (nTokens == 4);
	}

	bool const isValidPort(std::string const& str) {
		int portValue = std::stoi(str);
		return (portValue >= 1024 && portValue <= 65535);
	}

	std::vector<std::string> const splitIntoIPAndPort(std::string const& str) {
		char const delim = ':';

		std::string token;
		std::vector<std::string> tokens;

		std::istringstream iss(str);
		while (std::getline(iss, token, delim))
			tokens.push_back(token);

		if (tokens.size() != 2)
			throw BT::UnknownPeerExpressionException();

		if (!isValidIP(tokens[0]))
			throw BT::InvalidIPAddressException();

		if (!isValidPort(tokens[1]))
			throw BT::InvalidPortNumberException();

		return tokens;
	}

	void printApplicationConfigs(std::ostream& os) {
		os << "*** BitTorrent application configuration ***" << std::endl;
		os << "    Default log file name: " << BT::Defaults::defaultLogFilename << std::endl;
		os << "    Maximum internal buffer size: " << BT::Defaults::MaxBufferSize << std::endl;
		os << "    Maximum number of peer connections: " << BT::Defaults::MaxConnections << std::endl;
		os << "    Range of ports to use for reaching peers: " << BT::Defaults::InitPort << "-" << BT::Defaults::MaxPort << std::endl;
		os << "    Default port: " << BT::Defaults::DefaultPort << std::endl;
		os << "    Maximum supported size of internal buffers:" << std::endl;
		os << "        File name length: " << BT::Defaults::MaxFilenameSize << std::endl;
		os << "        Block length: " << BT::Defaults::BlockSize << std::endl;
		os << "        IP address size: " << BT::Defaults::IPSize << std::endl;
		os << "        IP port size: " << BT::Defaults::PortSize << std::endl;
		os << "        SHA1 Hash size: " << BT::Defaults::Sha1MdSize << std::endl;
	}
}

namespace BT {
	std::ostream& operator<<(std::ostream& os, BT::Config_t const& config) {
		printApplicationConfigs(os);

		os << "*** BitTorrent user configuration ***" << std::endl;
		os << "    .torrent file: " << config.getTorrentFilename() << std::endl;
		os << "    Save file: " << config.getSaveFilename() << std::endl;
		os << "    Log file: " << config.getLogFilename() << std::endl;

		auto isSeeder = [&]() { return config.getPeers().empty(); };
		if (isSeeder()) {
			os << "    Seeder's port: " << config.getSeederPort() << std::endl;
		}
		else {
			os << "    Leecher's peers information: " << std::endl;
			for (BT::Peer_t const& peer : config.getPeers())
				os << "    " << peer.getId() << "\t\t" << peer.getIp() << ":" << peer.getPort() << std::endl;
		}

		return os;
	}

	/* TODO: constructor doing heavy work :( */
	BT::Config_t::Config_t(std::vector<std::string> const& args)
		: helpRequested(false), enableVerbose(false), clientId(0), seederPort(BT::Defaults::DefaultPort),
		saveFilename(""), logFilename(BT::Defaults::defaultLogFilename), torrentFilename("") {

		helpRequested = (find(args.cbegin(), args.cend(), "-h") != args.cend());
		auto throwException = [&](std::exception const& e) { if (!helpRequested) throw e; };
		
		auto isOptionArg = [](std::string const& arg) { return (arg == "-h") || (arg == "-v"); };
		std::map<std::string, std::function<void()>> optionArgHandler;
		optionArgHandler["-h"] = [&]() { helpRequested = true; };
		optionArgHandler["-v"] = [&]() { enableVerbose = true; };


		auto isKeyValueArg = [](std::string const& arg) { 
			std::vector<std::string> const keyValueArgs{"-s", "-l", "-I", "-b", "-p"};
			return (std::find(keyValueArgs.cbegin(), keyValueArgs.cend(), arg) != keyValueArgs.cend());
		};
		std::map<std::string, std::function<void(std::string const&)>> keyValueArgHandler;
		keyValueArgHandler["-s"] = [&](std::string const& value) { saveFilename = value; };
		keyValueArgHandler["-l"] = [&](std::string const& value) { logFilename = value; };
		keyValueArgHandler["-I"] = [&](std::string const& value) { clientId = std::stoi(value); };
		keyValueArgHandler["-b"] = [&](std::string const& value) { 
			if (isValidPort(value)) seederPort = std::stoi(value);
			else throwException(BT::InvalidPortNumberException());
		};
		keyValueArgHandler["-p"] = [&](std::string const& value) {
			if (peersIPAndPortInfoList.size() < BT::Defaults::MaxConnections) peersIPAndPortInfoList.push_back(value);
			else throwException(BT::PeerLimitExceededException());
		};

		for (unsigned int i = 0; i < args.size(); i++) {
			if (isKeyValueArg(args[i])) {
				if (i + 1 < args.size() && args[i+1][0] != '-') {
					auto handler = keyValueArgHandler.find(args[i]);
					if (handler != keyValueArgHandler.end()) {
						i++;
						handler->second(args[i]);
						continue;
					}
				}
			}
			else if (isOptionArg(args[i])) {
				auto handler = optionArgHandler.find(args[i]);
				if (handler != optionArgHandler.end()) {
					handler->second();
					continue;
				}
			}
			
			if (args[i][0] != '-' && torrentFilename.empty()) {
				torrentFilename = args[i];
				continue;
			}

			throwException(BT::UnknownOptionException());
		}

		if (torrentFilename.empty()) throwException(BT::TorrentFileMissingException());
		if (saveFilename.empty()) saveFilename = torrentFilename;
	}
}

BT::PeersList_t BT::Config_t::getPeers() const {
	BT::PeersList_t peers;

	for (auto peerIPAndPort : peersIPAndPortInfoList) {
		auto tokens = splitIntoIPAndPort(peerIPAndPort);
		peers.push_back(BT::Peer_t(tokens[0], std::stoi(tokens[1])));
	}

	return std::move(peers);
}