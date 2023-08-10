#if !defined(CONNECTED_SOCKET_PARCEL_HPP)
#define CONNECTED_SOCKET_PARCEL_HPP

#include <string>

namespace BT 
{
    struct ConnectedSocketParcel
    {
        int connectedSockfd;
        std::string toIp;
        unsigned int toPort;
        
        ConnectedSocketParcel(int const sockfd, 
                              std::string const& ip, 
                              unsigned int const port)
            : connectedSockfd(sockfd),
              toIp(ip),
              toPort(port)
        {
        }
    };
}

#endif // !defined(CONNECTED_SOCKET_PARCEL_HPP)
