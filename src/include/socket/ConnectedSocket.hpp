#if !defined(CONNECTED_SOCKET_HPP)
#define CONNECTED_SOCKET_HPP

#include <string>

#include "socket/Socket.hpp"

namespace BT 
{
    struct ConnectedSocketParcel;
    class ConnectedSocket : public ISocket
    {
    private:
        ConnectedSocket() = default;

    public:
        ConnectedSocket(ConnectedSocketParcel const&);
        ConnectedSocket(ConnectedSocket const&) = delete;
        ConnectedSocket& operator=(ConnectedSocket const&) = delete;
        ConnectedSocket(ConnectedSocket&&);
        ConnectedSocket& operator=(ConnectedSocket&&);
        virtual ~ConnectedSocket();

        virtual int Send(void const * const, unsigned int) const;
        virtual int Receive(void*, unsigned int) const;

        virtual void Close();

    private:
        std::string fromIp;
        unsigned int fromPort;
        std::string toIp;
        unsigned int toPort;
        unsigned int connectedSockfd;
    };
}

#endif // !defined(CONNECTED_SOCKET_HPP)
