#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <algorithm>

#include "Peer.hpp"
#include "Torrent.hpp"

namespace BT 
{
	class StartParams_t 
	{
	public:
		StartParams_t(std::vector<std::string> const&);
		StartParams_t(StartParams_t const&) = default;
		StartParams_t(StartParams_t&&) = default;
		StartParams_t& operator=(StartParams_t const&) = default;
		StartParams_t& operator=(StartParams_t&&) = default;
		~StartParams_t() = default;

		bool IsSeeder() const;

		static std::string GetHelpMesssage();
		friend std::ostream& operator<<(std::ostream&, BT::StartParams_t const&);

	private:
		void throwException(std::exception const&) const;

		void initFlagOptionHandlers();
		void initKeyValueOptionsHandlers();
		void initOptionHandlers();
		void parseOptions(std::vector<std::string> const&, bool const);
		bool isFlagOption(std::string const&) const;
		bool isKeyValueOption(std::string const&) const;

	public:
		bool helpRequested;
		bool enableVerbose;
		unsigned int clientId;
		unsigned int seederPort;
		std::string saveFilename;
		std::string logFilename;
		std::string torrentFilename;
		PeersList_t peers;

	private:
		std::map<std::string, std::function<void()>> flagOptionHandler;
		std::map<std::string, std::function<void(std::string const&)>> keyValueOptionHandler;
	};

	std::ostream& operator<<(std::ostream& os, BT::StartParams_t const&);
}

#endif
