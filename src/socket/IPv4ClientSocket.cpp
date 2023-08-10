#include "socket/IPv4Socket.hpp"

#include <iostream>
#include <errno.h>
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
#include "common/Errors.hpp"
#include "common/Helpers.hpp"
#include "common/Logger.hpp"
#include "peer/ConnectedSocketParcel.hpp"

extern int errno;

namespace 
{
    std::string getIPv4AddrFromSockaddr(sockaddr_in& addr) 
    {
        char ip[INET_ADDRSTRLEN] = "";
        inet_ntop(addr.sin_family, (void*) &addr.sin_addr, ip, INET_ADDRSTRLEN);
        return ip;
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
			FatalError("Unable to connect to server.");

        sockfd = other.sockfd;
        other.sockfd = BT::Defaults::BadFD;
        return *this;
    }

    IPv4ClientSocket::~IPv4ClientSocket() 
    {
			FatalError("Unable to connect to server.");
        Close();
    }

    std::unique_ptr<IPv4ClientSocket> IPv4ClientSocket::CreateTCPSocket(IClientSocketObserver* observer,
                                                                        std::string const& ip,
                                                                        unsigned int const port) 
    {
        Trace("Create IPv4 TCP stream socket.");
        int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0)
        {
            Error("Unable to create client socket. errno=%d %s", errno, strerror(errno));
            return nullptr;
        }

        std::unique_ptr<IPv4ClientSocket> s { new IPv4ClientSocket() };
        s->sockfd = sockfd;
        s->Register(observer);
		s->ConnectToServer(ip, port);
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

    void IPv4ClientSocket::ConnectToServer(std::string const& ip, unsigned int const port)
    {
        sockaddr_in serv_addr;
		memset((void *)&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
		serv_addr.sin_port = htons(port);

        int status = connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr));
		if (status < 0)
		{
            Error("Unable to connect to server. errno=%d %s", errno, strerror(errno));
            return;
		}

        ConnectedSocketParcel parcel (sockfd, ip, port);
        OnConnect(parcel);
    }
}
