#if !defined(CLIENT_SOCKET_OBSERVABLE_HPP)
#define CLIENT_SOCKET_OBSERVABLE_HPP

#include <vector>

#include "socket/ClientSocket.hpp"

namespace BT 
{
    struct ConnectedSocketParcel;
    class IClientSocketObserver;
    class IClientSocketObservable : public IClientSocket
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
