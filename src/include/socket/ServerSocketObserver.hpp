#if !defined(SERVER_SOCKET_OBSERVER_HPP)
#define SERVER_SOCKET_OBSERVER_HPP

#include <string>

namespace BT 
{
    struct ConnectedSocketParcel
    {
        std::string mIP;
        std::string mPort;
        std::string mSockfd;
    };

    class IServerSocketObservable;
    class IServerSocketObserver 
    {
    public:
        IServerSocketObserver() = default;
        IServerSocketObserver(IServerSocketObservable* observablePtr);
        virtual ~IServerSocketObserver();

        // Callback that must be invoked by IServerSocketObservable
        virtual void OnClientConnected(ConnectedSocketParcel const&) const = 0;

    private:
        IServerSocketObservable* mObservablePtr;
    };
}

#endif // !defined(SERVER_SOCKET_OBSERVER_HPP)