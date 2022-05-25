#if !defined(SERVER_SOCKET_HPP)
#define SERVER_SOCKET_HPP

#include "socket/Socket.hpp"

namespace BT 
{
    class IServerSocket : public ISocket
    {
    public:
        virtual void AcceptClients(unsigned int const) = 0;
    };
}

#endif // defined(SERVER_SOCKET_HPP)
