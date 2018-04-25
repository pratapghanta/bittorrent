#ifndef PEER_HPP
#define PEER_HPP

#include <string>
#include <vector>

#include "Message.hpp"

namespace BT {

	/* Using the Peer class                                                                */
	/*                                                                                     */
	/* 1. Copy is prohibited                                                               */
	/* Each Leecher/Seeder will be associated with a single socket for communication       */
	/* It doesn't make sense to have multiple Peers using the same socket.                 */
	/* Notes:                                                                              */
	/*     a. The move operations are permitted, as the socket is reset after move.        */
	/*     b. The parameter to operator= is not a reference. This forces assignments using */
	/*        only r-values.                                                               */

	class Peer_t;
	using PeersList_t = std::vector<Peer_t>;

	class Peer_t {
	public:
		Peer_t(int const&, std::string const&, unsigned int const&);
		Peer_t(std::string const&, unsigned int const&);

		Peer_t(Peer_t const& p) = delete;
		Peer_t(Peer_t&& p);
		Peer_t& operator=(Peer_t p); 

		~Peer_t();

		int const getSockfd() const { return sockfd; }
		std::string const& getIp() const { return ip; }
		unsigned int const getPort() const { return port; }
		std::string const& getId() const { return id; }

		void receive(void *buf, unsigned int const count) const;
		void send(void const * const buf, unsigned int const count) const;

		Message_t const receiveMessage(Message_t::MessageType const type) const;
		void sendMessage(Message_t const& msg) const;

	private:
		void reset();

	private:
		int sockfd;
		std::string ip;
		unsigned int port;
		std::string id;
	};
}
#endif
