#if !defined(IPV4SOCKET_HPP)
#define IPV4SOCKET_HPP

#include <memory>
#include <string>

#include "socket/ServerSocketObservable.hpp"
#include "socket/ClientSocketObservable.hpp"

namespace BT
{
    class IPv4ServerSocket : public IServerSocketObservable
    {
    private:
        IPv4ServerSocket();

    public:
        static std::unique_ptr<IPv4ServerSocket> CreateTCPSocket(unsigned int const port);

        IPv4ServerSocket(IPv4ServerSocket const&) = delete;
        IPv4ServerSocket& operator=(IPv4ServerSocket const&) = delete;
        IPv4ServerSocket(IPv4ServerSocket&&);
        IPv4ServerSocket& operator=(IPv4ServerSocket&&);
        ~IPv4ServerSocket();

        virtual void Close();
        virtual void AcceptConnection();

    private:
        int listenSockfd;
        std::string ip;
        unsigned int listenPort;
    };

    class IPv4ClientSocket : public IClientSocketObservable
    {
    private:
        IPv4ClientSocket();

    public:
        static std::unique_ptr<IPv4ClientSocket> CreateTCPSocket();

        IPv4ClientSocket(IPv4ClientSocket const&) = delete;
        IPv4ClientSocket& operator=(IPv4ClientSocket const&) = delete;
        IPv4ClientSocket(IPv4ClientSocket&&);
        IPv4ClientSocket& operator=(IPv4ClientSocket&&);
        ~IPv4ClientSocket();

        virtual void Close();
        virtual void ConnectToServer(std::string const& ip, unsigned int port);

    private:
        int sockfd;
    };
}

#endif // !defined(IPV4SOCKET_HPP)
