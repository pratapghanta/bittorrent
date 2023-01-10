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

    std::string getIPv4AddrFromSocket(int const sockfd) 
    {
        if (sockfd == BT::Defaults::BadFD) {
            return "";
        }

        sockaddr_in sin;        
        socklen_t len = sizeof(sockaddr_in);
        memset((void*) &sin, 0, sizeof(sin));
        if (getsockname(sockfd, (sockaddr*) &sin, &len) == -1) 
        {
            PrintErrorAndExit("Unable to determine IP address associated with the socket.");            
        }

        return getIPv4AddrFromSockaddr(sin);
    }

    void bindIPv4Socket(int const sockfd, unsigned int const port) 
    {
        sockaddr_in sin;
        memset((void*) &sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = htonl(INADDR_ANY);
        sin.sin_port = htons(port);

        if (::bind(sockfd, (sockaddr*) &sin, sizeof(sin)) != 0) 
        {
            PrintErrorAndExit("Socket binding failed.");
        }
    }
}

namespace BT 
{
    IPv4ServerSocket::IPv4ServerSocket()
        : listenSockfd(BT::Defaults::BadFD),
          listenPort(0) {}

    IPv4ServerSocket::IPv4ServerSocket(IPv4ServerSocket&& other)
        : listenSockfd(other.listenSockfd),
          listenPort(other.listenPort) 
    {
        other.listenSockfd = BT::Defaults::BadFD;
    }

    IPv4ServerSocket& IPv4ServerSocket::operator=(IPv4ServerSocket&& other) 
    {
        if (this == &other) 
        {
            return *this;
        }

        listenSockfd = other.listenSockfd;
        listenPort = other.listenPort;
        other.listenSockfd = BT::Defaults::BadFD;
        return *this;
    }

    IPv4ServerSocket::~IPv4ServerSocket() 
    {
        Close();
    }

    std::unique_ptr<IPv4ServerSocket> IPv4ServerSocket::CreateTCPSocket(unsigned int const port) 
    {
        std::unique_ptr<IPv4ServerSocket> s { new IPv4ServerSocket() };
        s->listenSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        s->ip = getIPv4AddrFromSocket(s->listenSockfd);
        s->listenPort = port;

        bindIPv4Socket(s->listenSockfd, s->listenPort);
        listen(s->listenSockfd, Defaults::MaxConnections);

        return s;
    }

    void IPv4ServerSocket::Close() 
    {
        if (listenSockfd > 0) 
        {
            close(listenSockfd);
        }
        listenSockfd = BT::Defaults::BadFD;
    }

    void IPv4ServerSocket::AcceptConnection() 
    {
        unsigned int connections = 0;
        while (connections < Defaults::MaxConnections)
        {
            sockaddr_in client_addr;
            socklen_t clilen = sizeof(client_addr);
            int connectedSockfd = accept(listenSockfd, (sockaddr *) &client_addr, &clilen);
            if (connectedSockfd == Defaults::BadFD)
            {
                PrintErrorAndExit("Unable to accept a connection.");
            }
        
            ConnectedSocketParcel parcel;
            parcel.fromIp = ip;
            parcel.fromPort = listenPort;
            parcel.toIp = getIPv4AddrFromSockaddr(client_addr);
            parcel.toPort = ntohs(client_addr.sin_port);
            parcel.connectedSockfd = connectedSockfd;
            OnAcceptConnection(parcel);

            connections++;    
        }
    }
}
