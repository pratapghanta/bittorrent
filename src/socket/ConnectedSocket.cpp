#include "socket/ConnectedSocket.hpp"

#include <iostream>
#include <string>
#include <algorithm>
#include <exception>
#include <utility>
#include <cstring>
#include <ctime>
#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <functional>
#include <netinet/in.h>
#include <netinet/ip.h> //ip hdeader library (must come before ip_icmp.h)
#include <netinet/ip_icmp.h> //icmp header
#include <arpa/inet.h> //internet address library
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <openssl/sha.h>

#include "common/Defaults.hpp"
#include "common/Errors.hpp"
#include "common/Helpers.hpp"
#include "peer/MessageParcelFactory.hpp"
#include "socket/ConnectedSocketParcel.hpp"

namespace
{
    struct SocketNonBlockable : BT::CException 
	{
		SocketNonBlockable()
            : BT::CException("Unable to make socket non-blockable.") 
        {}
	};

    void makeSocketNonBlockable(int sockfd) 
	{
		int flags = fcntl(sockfd, F_GETFL, 0);
		if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK))
        {
            // TODO: Deal with this error without throwing exceptions??
			throw SocketNonBlockable();
        }
	}
}
namespace BT 
{
    ConnectedSocket::ConnectedSocket(ConnectedSocketParcel const& parcel)
        : fromIp(parcel.fromIp),
          fromPort(parcel.fromPort),
          toIp(parcel.toIp),
          toPort(parcel.toPort),
          connectedSockfd(dup(parcel.connectedSockfd))
    {
        makeSocketNonBlockable(connectedSockfd);
    }

    ConnectedSocket::ConnectedSocket(ConnectedSocket&& other)
        : fromIp(other.fromIp),
          fromPort(other.fromPort),
          toIp(other.toIp),
          toPort(other.toPort),
          connectedSockfd(other.connectedSockfd)
    {
        other.connectedSockfd = Defaults::BadFD;
    }

    ConnectedSocket& ConnectedSocket::operator=(ConnectedSocket&& other)
    {
        if (this == &other) 
        {
            return *this;
        }

        fromIp = other.fromIp;
        fromPort = other.fromPort;
        toIp = other.toIp;
        toPort = other.toPort;
        connectedSockfd = other.connectedSockfd;
        other.connectedSockfd = BT::Defaults::BadFD;
        
        return *this;
    }
    
    ConnectedSocket::~ConnectedSocket()
    {
        Close();
    }

    int ConnectedSocket::Send(char const * const buffer, unsigned int count) const
    {
        return write(connectedSockfd, buffer, count);
    }

    int ConnectedSocket::Receive(char* buffer, unsigned int count) const
    {
        memset(buffer, 0, count);
		
		time_t beg = time(0);
        unsigned int totalBytesRead = 0;
		while (totalBytesRead < count) 
        {
			auto bufPtr = &(static_cast<char*>(buffer)[totalBytesRead]);
			int const bytesRead = read(connectedSockfd, bufPtr, 1);
			if (bytesRead > 0) 
            {
				totalBytesRead += bytesRead;
				time(&beg);
				continue;
			}

			time_t end = time(0);
			if (difftime(end, beg) > 2 * 60)
            {
                // TODO: How to deal with significant delay?
            }
		}

        return totalBytesRead;
    }

    void ConnectedSocket::Close()
    {
        if (connectedSockfd > 0) 
        {
            close(connectedSockfd);
        }
        connectedSockfd = BT::Defaults::BadFD;
    }
}
