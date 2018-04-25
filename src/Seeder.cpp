#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <openssl/sha.h>

#include "Seeder.hpp"
#include "Peer.hpp"
#include "Defaults.hpp"
#include "helpers.hpp"

using namespace std;

namespace {
	int const createServerSocket(unsigned int const port) {
		int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); /* TCP */
		if (sockfd == BT::Defaults::BadSocketFD)
			throwErrorAndExit("Socket creation failed.");

		sockaddr_in server_addr;
		memset((void *)&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET; /* IPV4 */
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons(port);

		if (bind(sockfd, (sockaddr *)& server_addr, sizeof(server_addr)) != 0)
			throwErrorAndExit("Socket binding failed.");

		return sockfd;
	}

	std::string getIPFromSockAddr(sockaddr_in const& addr) {
		char ip[BT::Defaults::IPSize] = "";
		
		inet_ntop(addr.sin_family, (void *)&addr.sin_addr, ip, BT::Defaults::IPSize);
		
		return std::string(ip);
	}

	std::string getSeederIP(int const sockfd) {
		sockaddr_in sin;		
		memset((void *)&sin, 0, sizeof(sin));

		socklen_t len = sizeof(sockaddr_in);

		if (getsockname(sockfd, (sockaddr *)&sin, &len) != -1)
			return getIPFromSockAddr(sin);

		throwErrorAndExit("Unable to get seeder IP.");
		return std::string(""); // Unreachable. Might not have RVO :(
	}
}

BT::Seeder_t::Seeder_t(Torrent_t const& t, unsigned int const p) : sockfd(BT::Defaults::BadSocketFD), torrent(t), port(p) {
	leecherHandlers.reserve(BT::Defaults::MaxConnections);
}

void BT::Seeder_t::startTransfer(void) {
    sockfd = createServerSocket(port);
	std::string const& seederIP = getSeederIP(sockfd);
	
	listen(sockfd, BT::Defaults::MaxConnections);

    unsigned int nPeers = 0;
    while(nPeers < BT::Defaults::MaxConnections) {
		sockaddr_in client_addr;
        socklen_t clilen = sizeof(client_addr);

        int leecherfd = accept(sockfd, (sockaddr *) &client_addr, &clilen);
		if (leecherfd == BT::Defaults::BadSocketFD)
			throwErrorAndExit("Unable to connect to leecher");

		nPeers++;

    	std::string const& leecherIP = getIPFromSockAddr(client_addr);
        unsigned int leecherPort = ntohs(client_addr.sin_port);
        
		Peer_t leecher(leecherfd, leecherIP, leecherPort);
		Peer_t seeder(BT::Defaults::BadSocketFD, seederIP, port); /* hack */

		LeecherHandler_t lh(torrent, seeder, leecher);
		leecherHandlers.push_back(std::move(lh));
        /* (*leecherHandler[nPeers]).transfer(); */
        /* thread sth(&LeecherHandler::transfer, (*leecherHandler[nPeers])); */ /* ??????? */
                                            /* Start a thread for handlind the leecher ? */
                                            /* What is this stupid syntax ?              */
    }
}

BT::Seeder_t::~Seeder_t() {
	if (sockfd > 0)
		close(sockfd);
}
