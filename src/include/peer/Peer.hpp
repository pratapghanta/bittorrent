#if !defined(PEER_HPP)
#define PEER_HPP

#include <string>
#include <vector>

namespace BT 
{
	class Peer;
	using PeersList = std::vector<Peer>;

	class Peer 
	{
	public:
		Peer();
		Peer(unsigned int const);
		Peer(std::string const&, unsigned int const);
		Peer(Peer const& p) = default;
		Peer& operator=(Peer const&) = default; 
		Peer(Peer&& p) = default;
		Peer& operator=(Peer&&) = default; 
		~Peer() = default;

		friend std::ostream& operator<<(std::ostream& os, BT::Peer const&);

	public:
		std::string ip;
		unsigned int port;
	};

	std::ostream& operator<<(std::ostream& os, BT::Peer const&);
}

#endif // !defined(PEER_HPP)
