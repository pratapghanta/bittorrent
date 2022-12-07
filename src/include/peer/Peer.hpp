#if !defined(PEER_HPP)
#define PEER_HPP

#include <string>
#include <vector>

#include "MessageParcel.hpp"

namespace BT 
{
	class Peer;
	using PeersList = std::vector<Peer>;

	class Peer 
	{
	public:
		Peer();
		Peer(int const&, std::string const&, unsigned int const&);
		Peer(std::string const&, unsigned int const&);
		Peer(Peer const& p);
		Peer(Peer&& p);
		Peer& operator=(Peer); 
		~Peer();

		void EstablishConnectionTo(Peer const& otherPeer);

		void Receive(void *buf, unsigned int const count) const;
		void Send(void const * const buf, unsigned int const count) const;

		MessageParcel const ReceiveMessage(MessageType const type) const;
		void SendMessage(MessageParcel const& msg) const;

		std::string GetId() { return id; }

		friend std::ostream& operator<<(std::ostream& os, BT::Peer const&);
		friend void swap(Peer& first, Peer& second);

	private:
		void reset();

	private:
		int sockfd;
		std::string ip;
		unsigned int port;
		std::string id;
	};

	void swap(Peer& first, Peer& second);
	std::ostream& operator<<(std::ostream& os, BT::Peer const&);
}

#endif // !defined(PEER_HPP)
