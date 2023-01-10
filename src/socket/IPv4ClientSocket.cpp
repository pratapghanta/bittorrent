#include "socket/IPv4Socket.hpp"

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
#include "socket/ConnectedSocketParcel.hpp"

namespace 
{
    std::string getIPv4AddrFromSockaddr(sockaddr_in& addr) 
    {
        char ip[BT::Defaults::IPSize] = "";
        inet_ntop(addr.sin_family, (void*) &addr.sin_addr, ip, BT::Defaults::IPSize);
        return ip;
    }

    void getIPAndPortFromSocket(/* IN */  int const sockfd,
                                /* OUT */ std::string& ip,
                                /* OUT */ unsigned int& port) 
    {
        if (sockfd == BT::Defaults::BadFD) 
        {
            return;
        }

        sockaddr_in sin;        
        socklen_t len = sizeof(sockaddr_in);
        memset((void*) &sin, 0, sizeof(sin));
        if (getsockname(sockfd, (sockaddr*) &sin, &len) == -1) 
        {
            PrintErrorAndExit("Unable to determine IP address associated with the socket.");            
        }

        ip = getIPv4AddrFromSockaddr(sin);
        port = ntohs(sin.sin_port);
    }
}

namespace BT 
{
    IPv4ClientSocket::IPv4ClientSocket()
        : sockfd(BT::Defaults::BadFD) {}

    IPv4ClientSocket::IPv4ClientSocket(IPv4ClientSocket&& other)
        : sockfd(other.sockfd)
    {
        other.sockfd = BT::Defaults::BadFD;
    }

    IPv4ClientSocket& IPv4ClientSocket::operator=(IPv4ClientSocket&& other) 
    {
        if (this == &other) 
        {
            return *this;
        }

        sockfd = other.sockfd;
        other.sockfd = BT::Defaults::BadFD;
        return *this;
    }

    IPv4ClientSocket::~IPv4ClientSocket() 
    {
        Close();
    }

    std::unique_ptr<IPv4ClientSocket> IPv4ClientSocket::CreateTCPSocket() 
    {
        std::unique_ptr<IPv4ClientSocket> s { new IPv4ClientSocket() };
        s->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        return s;
    }

    void IPv4ClientSocket::Close() 
    {
        if (sockfd > 0) 
        {
            close(sockfd);
        }
        sockfd = BT::Defaults::BadFD;
    }

    void IPv4ClientSocket::ConnectToServer(std::string const& ip, unsigned int port)
    {
        sockaddr_in serv_addr;
		memset((void *)&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
		serv_addr.sin_port = htons(port);

		if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) != 0)
		{
			PrintErrorAndExit("Unable to connect to server.");
		}

        ConnectedSocketParcel parcel;
        getIPAndPortFromSocket(sockfd, parcel.fromIp, parcel.fromPort);
        parcel.toIp = ip;
        parcel.toPort = port;
        parcel.connectedSockfd = sockfd;
        sockfd = BT::Defaults::BadFD;
        OnConnect(parcel);
    }
}
