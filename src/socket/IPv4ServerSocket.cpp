#include "socket/IPv4Socket.hpp"

#include "common/Defaults.hpp"
#include "common/Helpers.hpp"

#if defined(_DEBUG)
#define DEBUG_IPV4_SERVER_SOCKET
#endif // defined(_DEBUG)

namespace {
    std::string getIPv4AddrFromSockaddr(sockaddr_in& sin) {
        char ip[BT::Defaults::IPSize] = "";
        inet_ntop(addr.sin_family, (void*) &addr.sin_addr, ip, BT::Defaults::IPSize);
        return ip;
    }

    std::string getIPv4AddrFromSocket(int const sockfd) {
        if (sockfd == BT::Defaults::BadFD) {
            return "";
        }

        sockaddr_in sin;        
        socklen_t len = sizeof(sockaddr_in);
        memset((void*) &sin, 0, sizeof(sin));
        if (getsockname(sockfd, (sockaddr*) &sin, &len) == -1) {
            ThrowErrorAndExit("Unable to determine IP address associated with the socket.");            
        }

        return getIPv4AddrFromSockaddr(sin);
    }

    void bindSocket(int const sockfd) {
        sockaddr_in server_addr;
        memset((void*) &server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(mListeningPort);

        if (::bind(sockfd, (sockaddr*) &server_addr, sizeof(server_addr)) != 0) {
            ThrowErrorAndExit("Socket binding failed.");
        }
    }
}

namespace BT {
    IPv4ServerSocket::IPv4ServerSocket()
        : mSockfd(BT::Defaults::BadFD),
          mListeningPort(0) {}

    IPv4ServerSocket(IPv4ServerSocket&& other)
        : mSockfd(BT::Defaults::BadFD) {
        mSockfd = other.mSockfd;
        mListeningPort = other.mListeningPort;

        other.mSockfd = BT::Defaults::BadFD;
    }

    IPv4ServerSocket& operator=(IPv4ServerSocket&& other) {
        if (this == &other) {
            return *this;
        }

        mSockfd = other.mSockfd;
        mListeningPort = other.mListeningPort;

        other.mSockfd = BT::Defaults::BadFD;
    }

    ~IPv4ServerSocket() {
        Close();
    }

    IPv4ServerSocket& IPv4ServerSocket::CreateTCPSocket(unsigned int port) {
        mListeningPort = port;
        mSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        mIP = getIPv4AddrFromSocket(mSockfd);

        bindSocket(mSockfd);

#if defined(DEBUG_IPV4_SERVER_SOCKET)
        Trace("Server: IP: %s, Port: %u, sockfd: %d", mIP.c_str(), mListeningPort, mSockfd);
#endif // defined(DEBUG_IPV4_SERVER_SOCKET)

        return *this;
    }

    void IPv4ServerSocket::AcceptClients(unsigned int const maxConnections) {
        listen(sockfd, maxConnections);

        unsigned int nPeers = 0;
        while(nPeers < maxConnections) {
            sockaddr_in client_addr;
            socklen_t clilen = sizeof(client_addr);
            int peerfd = accept(mSockfd, (sockaddr *) &client_addr, &clilen);
            if (peerfd == Defaults::BadFD)
                ThrowErrorAndExit("Unable to connect to leecher.");

            nPeers++;

            std::string const& peerIP = getIPv4AddrFromSocket(client_addr);
            unsigned int peerPort = ntohs(client_addr.sin_port);

            // notify observers about the incoming client and forget about it.
        }
    }

    virtual void Send() {}

    virtual void Receive() {}

    virtual void Close() {
        if (mSockfd > 0) {
            close(mSockfd);
        }
        mSockfd = BT::Defaults::BadFD;
    }
}
