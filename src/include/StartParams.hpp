#if !defined(START_PARAMS_HPP)
#define START_PARAMS_HPP

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <algorithm>

#include "peer/Peer.hpp"
#include "torrent/Torrent.hpp"

namespace BT 
{
	class StartParams 
	{
	public:
		StartParams(std::vector<std::string> const&);
		StartParams(StartParams const&) = default;
		StartParams(StartParams&&) = default;
		StartParams& operator=(StartParams const&) = default;
		StartParams& operator=(StartParams&&) = default;
		~StartParams() = default;

		bool IsSeeder() const;

		friend std::ostream& operator<<(std::ostream&, BT::StartParams const&);

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
		std::unordered_map<std::string, std::function<void()>> flagOptionHandler;
		std::unordered_map<std::string, std::function<void(std::string const&)>> keyValueOptionHandler;
	};

	std::ostream& operator<<(std::ostream& os, BT::StartParams const&);
}

#endif // !defined(START_PARAMS_HPP)
