#if !defined(START_PARAMS_HPP)
#define START_PARAMS_HPP

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

#include "common/StatusCode.hpp"
#include "peer/Peer.hpp"

namespace BT 
{
	struct StartParams 
	{
		StartParams(std::vector<std::string> const&, STATUSCODE&);
		~StartParams() = default;

		StartParams(StartParams const&) = default;
		StartParams& operator=(StartParams const&) = default;
		
		StartParams(StartParams&&) = default;
		StartParams& operator=(StartParams&&) = default;

		bool IsSeeder() const;
		friend std::ostream& operator<<(std::ostream&, StartParams const&);

		bool helpRequested;
		bool enableVerbose;
		unsigned int clientId;
		unsigned int seederPort;
		std::string saveFilename;
		std::string logFilename;
		std::string torrentFilename;
		PeersList peers;

	private:
		void throwException(std::exception const&) const;
		void initFlagOptionHandlers();
		void initKeyValueOptionsHandlers();
		void initOptionHandlers();
		void parseArgs(std::vector<std::string> const&);

		std::unordered_map<std::string, std::function<void()>> flagOptionHandler;
		std::unordered_map<std::string, std::function<void(std::string const&)>> keyValueOptionHandler;
	};

	std::ostream& operator<<(std::ostream& os, StartParams const&);
}

#endif // !defined(START_PARAMS_HPP)
