#include "socket/ClientSocketObserver.hpp"
#include "socket/ClientSocketObservable.hpp"

namespace BT
{
    IClientSocketObserver::IClientSocketObserver(IClientSocketObservable* observable)
        : observable(observable)
    {
        if (observable != nullptr)
        {
            observable->Register(this);
        }
    }

    IClientSocketObserver::~IClientSocketObserver()
    {
        if (observable != nullptr)
        {
            observable->Unregister(this);    
        }
    }
}
