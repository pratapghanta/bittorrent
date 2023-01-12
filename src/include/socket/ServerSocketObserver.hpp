#if !defined(SERVER_SOCKET_OBSERVER_HPP)
#define SERVER_SOCKET_OBSERVER_HPP

#include <string>

namespace BT 
{
    struct ConnectedSocketParcel;

    class IServerSocketObserver 
    {
    public:
        virtual void OnAcceptConnection(ConnectedSocketParcel const&) = 0;
    };
}

#endif // !defined(SERVER_SOCKET_OBSERVER_HPP)
