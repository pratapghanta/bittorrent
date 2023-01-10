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
		Peer(Peer const& p);
		Peer(Peer&& p);
		Peer& operator=(Peer); 
		~Peer() = default;

		std::string GetId() { return id; }

		friend std::ostream& operator<<(std::ostream& os, BT::Peer const&);
		friend void swap(Peer& first, Peer& second);

	private:
		void reset();

	public:
		std::string ip;
		unsigned int port;
		std::string id;
	};

	void swap(Peer& first, Peer& second);
	std::ostream& operator<<(std::ostream& os, BT::Peer const&);
}

#endif // !defined(PEER_HPP)
