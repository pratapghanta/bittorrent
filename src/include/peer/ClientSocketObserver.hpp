#if !defined(CLIENT_SOCKET_OBSERVER_HPP)
#define CLIENT_SOCKET_OBSERVER_HPP

namespace BT 
{
    struct ConnectedSocketParcel;

    class IClientSocketObserver 
    {
    public:
        virtual void OnConnect(ConnectedSocketParcel const&) = 0;
    };
}

#endif // !defined(CLIENT_SOCKET_OBSERVER_HPP)
