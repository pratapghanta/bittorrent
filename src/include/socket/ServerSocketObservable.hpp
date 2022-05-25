#if !defined(SERVER_SOCKET_OBSERVABLE_HPP)
#define SERVER_SOCKET_OBSERVABLE_HPP

#include "socket/ServerSocket.hpp"

namespace BT 
{
    class IServerSocketObserver;
    class IServerSocketObservable : public IServerSocket
    {
    public:
        virtual void Register(IServerSocketObserver const * const) = 0;
        virtual void Unregister() = 0;
        virtual void NotifyAll() = 0;
    };
}

#endif // !defined(SERVER_SOCKET_OBSERVABLE_HPP)
