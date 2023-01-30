#include <algorithm>

#include "peer/ConnectedSocketParcel.hpp"
#include "peer/ServerSocketObservable.hpp"
#include "peer/ServerSocketObserver.hpp"

namespace BT 
{
    void IServerSocketObservable::Register(IServerSocketObserver* observer)
    {
        auto iterator = std::find(observers.begin(), observers.end(), observer);
        if (iterator == observers.end()) 
        {
            observers.push_back(observer);
        }
    }

    void IServerSocketObservable::Unregister(IServerSocketObserver* observer)
    {
        auto iterator = std::find(observers.begin(), observers.end(), observer);
        if (iterator != observers.end()) 
        {
            observers.erase(iterator);
        }
    }

    void IServerSocketObservable::OnAcceptConnection(ConnectedSocketParcel const& parcel)
    {
        for (auto observer : observers)
        {
            observer->OnAcceptConnection(parcel);
        }
    }
}
