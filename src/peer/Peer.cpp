#include <iostream>

#include "peer/Peer.hpp"

namespace BT
{
	Peer::Peer()
		: port(0)
	{}

	Peer::Peer(unsigned int const port)
		: ip("localhost"),
		  port(port) 
	{}

	Peer::Peer(std::string const& ip, unsigned int const port)
		: ip(ip),
		  port(port) 
	{}

	std::ostream& operator<<(std::ostream& os, Peer const& peer) 
	{
		os << peer.ip << ":" << peer.port;
		return os;
	}
}