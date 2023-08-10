#if !defined(CLIENT_SOCKET_OBSERVABLE_HPP)
#define CLIENT_SOCKET_OBSERVABLE_HPP

#include <vector>

namespace BT 
{
    struct ConnectedSocketParcel;
    class IClientSocketObserver;

    class IClientSocketObservable
    {
    public:
        virtual void Register(IClientSocketObserver*);
        virtual void Unregister(IClientSocketObserver*);
        virtual void OnConnect(ConnectedSocketParcel const&);
    
    private:
        std::vector<IClientSocketObserver*> observers;
    };
}

#endif // !defined(CLIENT_SOCKET_OBSERVABLE_HPP)
