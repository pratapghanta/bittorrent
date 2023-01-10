#if !defined(CLIENT_SOCKET_OBSERVER_HPP)
#define CLIENT_SOCKET_OBSERVER_HPP

#include <string>

namespace BT 
{
    struct ConnectedSocketParcel;
    class IClientSocketObservable;
    class IClientSocketObserver 
    {
    public:
        IClientSocketObserver() = default;
        IClientSocketObserver(IClientSocketObservable* observable);
        virtual ~IClientSocketObserver();

        virtual void OnConnect(ConnectedSocketParcel const&) = 0;

    private:
        IClientSocketObservable* observable;
    };
}

#endif // !defined(CLIENT_SOCKET_OBSERVER_HPP)
