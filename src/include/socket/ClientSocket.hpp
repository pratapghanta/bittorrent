#if !defined(CLIENT_SOCKET_HPP)
#define CLIENT_SOCKET_HPP

#include <string>

#include "socket/Socket.hpp"

namespace BT 
{
    class IClientSocket : public ISocket 
    {
    public:
        virtual void ConnectToServer(std::string const& ip, unsigned int port) = 0;
    };
}

#endif // !defined(CLIENT_SOCKET_HPP)