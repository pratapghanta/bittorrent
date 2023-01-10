#if !defined(SERVER_SOCKET_OBSERVER_HPP)
#define SERVER_SOCKET_OBSERVER_HPP

#include <string>

namespace BT 
{
    struct ConnectedSocketParcel;
    class IServerSocketObservable;
    class IServerSocketObserver 
    {
    public:
        IServerSocketObserver() = default;
        IServerSocketObserver(IServerSocketObservable* observable);
        virtual ~IServerSocketObserver();

        virtual void OnAcceptConnection(ConnectedSocketParcel const&) = 0;

    private:
        IServerSocketObservable* observable;
    };
}

#endif // !defined(SERVER_SOCKET_OBSERVER_HPP)
