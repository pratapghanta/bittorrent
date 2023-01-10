#if !defined(SERVER_SOCKET_OBSERVABLE_HPP)
#define SERVER_SOCKET_OBSERVABLE_HPP

#include <vector>

#include "socket/ServerSocket.hpp"

namespace BT 
{
    struct ConnectedSocketParcel;
    class IServerSocketObserver;
    class IServerSocketObservable : public IServerSocket
    {
    public:
        virtual void Register(IServerSocketObserver*);
        virtual void Unregister(IServerSocketObserver*);
        virtual void OnAcceptConnection(ConnectedSocketParcel const&);
    
    private:
        std::vector<IServerSocketObserver*> observers;
    };
}

#endif // !defined(SERVER_SOCKET_OBSERVABLE_HPP)
