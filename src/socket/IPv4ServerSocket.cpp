#include "socket/IPv4Socket.hpp"

#include <iostream>
#include <fstream>
#include <cstring>
#include <errno.h>
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

    int bindIPv4Socket(int const sockfd, unsigned int const port)
    {
        sockaddr_in sin;
        memset((void*) &sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = htonl(INADDR_ANY);
        sin.sin_port = htons(port);

        return ::bind(sockfd, (sockaddr*) &sin, sizeof(sin));
    }

    void setSocketOptions(int const sockfd)
    {
        // Set the "LINGER" timeout to zero, to close the listen socket
        // immediately at program termination.
        //
        constexpr int LINGER_ACTIVE = 1;
        constexpr int LINGER_TIMEOUT = 0;
        struct linger linger_opt = { LINGER_ACTIVE, LINGER_TIMEOUT };
        setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
    }
}

namespace BT 
{
    IPv4ServerSocket::IPv4ServerSocket(int const sockfd,
                                       unsigned int const port,
                                       unsigned int const maxConnections)
        : listenSockfd(sockfd),
          listenPort(port),
          backlog(maxConnections) 
    {
    }

    IPv4ServerSocket::IPv4ServerSocket(IPv4ServerSocket&& other)
        : listenSockfd(other.listenSockfd),
          listenPort(other.listenPort),
          backlog(other.backlog) 
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
        backlog = other.backlog;

        other.listenSockfd = BT::Defaults::BadFD;
        return *this;
    }

    IPv4ServerSocket::~IPv4ServerSocket() 
    {
        Close();
    }

    std::unique_ptr<IPv4ServerSocket> IPv4ServerSocket::CreateTCPSocket(IServerSocketObserver* observer,
                                                                        unsigned int const port, 
                                                                        unsigned int maxConnections) 
    {
        Trace("Create IPv4 TCP stream socket.");
        int listenSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSockfd < 0)
        {
            Error("Unable to create server socket. errno=%d %s", errno, strerror(errno));
            return nullptr;
        }

        Trace("Bind: sockfd: %d\tport: %u", listenSockfd, port);
        int status = bindIPv4Socket(listenSockfd, port);
        if (status < 0)
        {
            Error("Unable to bind server socket %d to port %u. errno=%d %s", listenSockfd, port, errno, strerror(errno));
            return nullptr;
        }

        setSocketOptions(listenSockfd);

        Trace("Listen: sockfd: %d\tbacklog: %u", listenSockfd, maxConnections);
        status = listen(listenSockfd, maxConnections);
        if (status < 0)
        {
            Error("Unable to listen on server socket %d. errno=%d %s", listenSockfd, errno, strerror(errno));
            return nullptr;
        }

        std::unique_ptr<IPv4ServerSocket> s { new IPv4ServerSocket(listenSockfd, port, maxConnections) };
        s->Register(observer);   
        s->AcceptConnection();
        return s;
    }

    void IPv4ServerSocket::Close() 
    {
        if (listenSockfd > 0) 
        {
            close(listenSockfd);
            listenSockfd = BT::Defaults::BadFD;
        }
    }

    void IPv4ServerSocket::AcceptConnection() 
    {
        // TODO: 
        // Loop to accept multiple connections.
        // Ensure that the loop can be terminated.
        //
        sockaddr_in client_addr;
        socklen_t clilen = sizeof(client_addr);
        Trace("Block for Accept: sockfd: %d", listenSockfd);
        int connectedSockfd = accept(listenSockfd, (sockaddr *) &client_addr, &clilen);
        if (connectedSockfd == Defaults::BadFD)
        {
            BT::FatalError("Unable to accept a connection.");
        }
    
        ConnectedSocketParcel parcel(connectedSockfd,
                                     getIPv4AddrFromSockaddr(client_addr),
                                     ntohs(client_addr.sin_port));
        OnAcceptConnection(parcel);
    }
}
