#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <vector>
#include <string>

#include "Peer.hpp"

namespace BT {
	class Config_t {
	public:
		Config_t(std::vector<std::string> const& args);
		Config_t(Config_t const&) = delete;
		Config_t(Config_t&&) = delete;
		Config_t& operator=(Config_t const&) = delete;
		Config_t& operator=(Config_t&&) = delete;

		bool const isHelpRequested() const{ return helpRequested; }
		bool const isVerbose() const { return enableVerbose; }
		unsigned int const getClientId() const { return clientId; }
		unsigned int const getSeederPort() const { return seederPort; }
		std::string const& getSaveFilename() const { return saveFilename; }
		std::string const& getLogFilename() const { return logFilename; }
		std::string const& getTorrentFilename() const { return torrentFilename; }
		bool const isSeeder() const { return peersIPAndPortInfoList.empty(); }
		PeersList_t getPeers() const;

	private:
		bool helpRequested;
		bool enableVerbose;
		unsigned int clientId;
		unsigned int seederPort;
		std::string saveFilename;
		std::string logFilename;
		std::string torrentFilename;
		std::vector<std::string> peersIPAndPortInfoList;
	};

	std::ostream& operator<<(std::ostream& os, BT::Config_t const&);
}

#endif
