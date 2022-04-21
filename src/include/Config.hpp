#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

#include "Peer.hpp"
#include "Torrent.hpp"

namespace BT 
{
	class Config_t 
	{
	public:
		Config_t(std::vector<std::string> const&);
		Config_t(Config_t const&) = default;
		Config_t(Config_t&&) = default;
		Config_t& operator=(Config_t const&) = default;
		Config_t& operator=(Config_t&&) = default;
		~Config_t() = default;

		void StartPeer() const;
		friend std::ostream& operator<<(std::ostream&, BT::Config_t const&);

	private:
		void throwException(std::exception const&) const;

		void initFlagOptionHandlers();
		void initKeyValueOptionsHandlers();
		void initOptionHandlers();
		void parseOptions(std::vector<std::string> const&, bool const);
		bool isFlagOption(std::string const&) const;
		bool isKeyValueOption(std::string const&) const;
	
		bool isSeeder() const;
		void startSeeder(BT::Torrent_t const&) const;
		void startLeechers(BT::Torrent_t const&) const;
	
	private:
		bool helpRequested;
		bool enableVerbose;
		unsigned int clientId;
		unsigned int seederPort;
		std::string saveFilename;
		std::string logFilename;
		std::string torrentFilename;
		PeersList_t peers;

		std::map<std::string, std::function<void()>> flagOptionHandler;
		std::map<std::string, std::function<void(std::string const&)>> keyValueOptionHandler;
	};

	std::ostream& operator<<(std::ostream& os, BT::Config_t const&);
}

#endif
