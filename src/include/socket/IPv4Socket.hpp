#if !defined(IPV4SOCKET_HPP)
#define IPV4SOCKET_HPP

#include <memory>
#include <string>

#include "peer/ServerSocketObservable.hpp"
#include "peer/ServerSocketObserver.hpp"
#include "peer/ClientSocketObservable.hpp"
#include "peer/ClientSocketObserver.hpp"
#include "socket/ClientSocket.hpp"
#include "socket/ServerSocket.hpp"

namespace BT
{
    class IPv4ServerSocket : public IServerSocket, public IServerSocketObservable
    {
    private:
        IPv4ServerSocket(int const, unsigned int const, unsigned int const);

    public:
        static std::unique_ptr<IPv4ServerSocket> CreateTCPSocket(IServerSocketObserver*,
                                                                 unsigned int const,
                                                                 unsigned int const);

        IPv4ServerSocket(IPv4ServerSocket const&) = delete;
        IPv4ServerSocket& operator=(IPv4ServerSocket const&) = delete;
        IPv4ServerSocket(IPv4ServerSocket&&);
        IPv4ServerSocket& operator=(IPv4ServerSocket&&);
        virtual ~IPv4ServerSocket();

        virtual void Close();
        virtual void AcceptConnection();

    private:
        int listenSockfd;
        unsigned int listenPort;
        unsigned int backlog;
    };

    class IPv4ClientSocket : public IClientSocket, public IClientSocketObservable
    {
    private:
        IPv4ClientSocket();

    public:
        static std::unique_ptr<IPv4ClientSocket> CreateTCPSocket(IClientSocketObserver*,
                                                                 std::string const&,
                                                                 unsigned int const);

        IPv4ClientSocket(IPv4ClientSocket const&) = delete;
        IPv4ClientSocket& operator=(IPv4ClientSocket const&) = delete;
        IPv4ClientSocket(IPv4ClientSocket&&);
        IPv4ClientSocket& operator=(IPv4ClientSocket&&);
        virtual ~IPv4ClientSocket();

        virtual void Close();
        virtual void ConnectToServer(std::string const& ip, unsigned int const port);

    private:
        int sockfd;
    };
}

#endif // !defined(IPV4SOCKET_HPP)
