#ifndef PEER_HPP
#define PEER_HPP

#include <string>
#include <vector>

#include "Message.hpp"

namespace BT 
{
	class Peer_t;
	using PeersList_t = std::vector<Peer_t>;

	class Peer_t 
	{
	public:
		Peer_t();
		Peer_t(int const&, std::string const&, unsigned int const&);
		Peer_t(std::string const&, unsigned int const&);
		Peer_t(Peer_t const& p);
		Peer_t(Peer_t&& p);
		Peer_t& operator=(Peer_t); 
		~Peer_t();

		void EstablishConnectionTo(Peer_t const& otherPeer);

		void Receive(void *buf, unsigned int const count) const;
		void Send(void const * const buf, unsigned int const count) const;

		Message_t const ReceiveMessage(Message_t::MessageType const type) const;
		void SendMessage(Message_t const& msg) const;

		std::string GetId() { return id; }

		friend std::ostream& operator<<(std::ostream& os, BT::Peer_t const&);
		friend void swap(Peer_t& first, Peer_t& second);

	private:
		void Reset();

	private:
		int sockfd;
		std::string ip;
		unsigned int port;
		std::string id;
	};

	void swap(Peer_t& first, Peer_t& second);
	std::ostream& operator<<(std::ostream& os, BT::Peer_t const&);
}
#endif
