#include <iostream>
#include <fstream>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <openssl/sha.h>

#include "common/Defaults.hpp"
#include "common/Helpers.hpp"
#include "common/Logger.hpp"
#include "peer/Peer.hpp"
#include "peer/Seeder.hpp"

using namespace std;

namespace {
	int const createServerSocket() {
		int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); /* TCP */
		if (sockfd == BT::Defaults::BadFD)
			PrintErrorAndExit("Socket creation failed.");
		return sockfd;
	}

	void bindServerSocket(int sockfd, unsigned int const port) {
		sockaddr_in server_addr;
		memset((void*) &server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET; /* IPV4 */
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons(port);

		if (::bind(sockfd, (sockaddr*) &server_addr, sizeof(server_addr)) != 0)
			PrintErrorAndExit("Socket binding failed.");
	}

	std::string getIPFromSockAddr(sockaddr_in const& addr) {
		char ip[BT::Defaults::IPSize] = "";
		inet_ntop(addr.sin_family, (void*) &addr.sin_addr, ip, BT::Defaults::IPSize);
		return std::string(ip);
	}

	std::string getSeederIP(int const sockfd) {
		sockaddr_in sin;		
		memset((void*) &sin, 0, sizeof(sin));

		socklen_t len = sizeof(sockaddr_in);
		if (getsockname(sockfd, (sockaddr*) &sin, &len) != -1)
			return getIPFromSockAddr(sin);

		PrintErrorAndExit("Unable to get seeder IP.");
		return std::string(""); // Unreachable code. Might not have RVO :(
	}
}

namespace BT {
	Seeder_t::Seeder_t(Torrent const& t, unsigned int const p) 
		: sockfd(Defaults::BadFD), torrent(t), port(p) {
		leecherHandlers.reserve(Defaults::MaxConnections);
	}

	Seeder_t::Seeder_t(Seeder_t&& obj)
		: sockfd (obj.sockfd),
		  torrent(obj.torrent),
		  port(obj.port) {
		obj.sockfd = BT::Defaults::BadFD;
		obj.port = 0;
	}

	Seeder_t& Seeder_t::operator=(Seeder_t&& obj) {
		if (&obj == this)
			return *this;

		if (sockfd > 0)
			close(sockfd);

		sockfd = obj.sockfd;
		torrent = obj.torrent;
		port = obj.port;

		obj.sockfd = BT::Defaults::BadFD;
		obj.port = 0;

		return *this;
	}

	Seeder_t::~Seeder_t() {
		if (sockfd > 0)
			close(sockfd);
	}

	void Seeder_t::StartTransfer() {
		unsigned int maxThreadsPossible = std::thread::hardware_concurrency();
		if (maxThreadsPossible != 0 && maxThreadsPossible < Defaults::MaxConnections) {
			Warn("Using less leechers is recommended. Max concurrent threads supported = %u", maxThreadsPossible);
		}
		
		sockfd = createServerSocket();
		bindServerSocket(sockfd, port);

		listen(sockfd, Defaults::MaxConnections);

		std::string const& seederIP = getSeederIP(sockfd);
		unsigned int nPeers = 0;
		while(nPeers < Defaults::MaxConnections) {
			sockaddr_in client_addr;
			socklen_t clilen = sizeof(client_addr);

			int leecherfd = accept(sockfd, (sockaddr *) &client_addr, &clilen);
			if (leecherfd == Defaults::BadFD)
				PrintErrorAndExit("Unable to connect to leecher.");

			nPeers++;

			std::string const& leecherIP = getIPFromSockAddr(client_addr);
			unsigned int leecherPort = ntohs(client_addr.sin_port);
			
			Peer leecher(leecherfd, leecherIP, leecherPort);
			Peer seeder(Defaults::BadFD, seederIP, port); // hack

			LeecherHandler_t lh(torrent, seeder, leecher);
			leecherHandlers.push_back(std::move(lh));
			
			std::thread sth([&]() { leecherHandlers[nPeers-1].StartTransfer(); });
		}
	}
}