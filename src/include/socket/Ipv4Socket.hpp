#if !defined(IPV4SOCKET_HPP)
#define IPV4SOCKET_HPP

#include "socket/ServerSocketObservable.hpp"

namespace BT
{
    class IPv4ServerSocket : public IServerSocketObservable
    {
    private:
        IPv4ServerSocket();

    public:
        static IPv4ServerSocket& CreateTCPSocket(unsigned int port);
        // TODO: UDP sockets

        IPv4ServerSocket(IPv4ServerSocket const&) = delete;
        IPv4ServerSocket& operator=(IPv4ServerSocket const&) = delete;

        IPv4ServerSocket(IPv4ServerSocket&&);
        IPv4ServerSocket& operator=(IPv4ServerSocket&&);
        ~IPv4ServerSocket();

        virtual void AcceptClients(unsigned int const);

        virtual void Send(char*, unsigned int);
        virtual void Receive(char*, unsigned int);
        virtual void Destroy();

    private:
        int mListeningPort;

        int mSockfd;
        std::string mIP;
    };

    class IPv4ServerSocketObserver;
    class IPv4ServerSocketLoggingObserver;
}

#endif // !defined(IPV4SOCKET_HPP)
