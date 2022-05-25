#include "socket/ServerSocketObserver.hpp"
#include "socket/ServerSocketObservable.hpp"

namespace BT
{
    IServerSocketObserver::IServerSocketObserver(IServerSocketObservable* observablePtr)
        : mObservablePtr(observablePtr)
    {
        if (mObservablePtr == nullptr)
            return;

        mObservablePtr->Register(this);
    }

    IServerSocketObserver::~IServerSocketObserver()
    {
        if (mObservablePtr == nullptr)
            return;

        mObservablePtr->Unregister();    
        mObservablePtr = nullptr;
    }
}
