#include "socket/ClientSocketObservable.hpp"

#include <algorithm>

#include "socket/ClientSocketObserver.hpp"
#include "socket/ConnectedSocketParcel.hpp"

namespace BT 
{
    void IClientSocketObservable::Register(IClientSocketObserver* observer)
    {
        auto iterator = std::find(observers.begin(), observers.end(), observer);
        if (iterator == observers.end()) 
        {
            observers.push_back(observer);
        }
    }

    void IClientSocketObservable::Unregister(IClientSocketObserver* observer)
    {
        auto iterator = std::find(observers.begin(), observers.end(), observer);
        if (iterator != observers.end()) 
        {
            observers.erase(iterator);
        }
    }

    void IClientSocketObservable::OnConnect(ConnectedSocketParcel const& parcel)
    {
        for (auto observer : observers)
        {
            observer->OnConnect(parcel);
        }
    }
}
