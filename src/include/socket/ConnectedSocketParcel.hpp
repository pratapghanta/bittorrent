#if !defined(CONNECTED_SOCKET_PARCEL_HPP)
#define CONNECTED_SOCKET_PARCEL_HPP

#include <string>

namespace BT 
{
    struct ConnectedSocketParcel
    {
        std::string fromIp;
        unsigned int fromPort;
        std::string toIp;
        unsigned int toPort;
        int connectedSockfd;
    };
}

#endif // !defined(CONNECTED_SOCKET_PARCEL_HPP)
