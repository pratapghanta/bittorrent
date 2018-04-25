#ifndef SEEDER_HPP
#define SEEDER_HPP

#include <vector>

#include "Config.hpp"
#include "Torrent.hpp"
#include "Peer.hpp"

namespace BT {
	class Seeder_t {
	public:
		Seeder_t(Torrent_t const& t, unsigned int const p);
		~Seeder_t();

		unsigned int const getPort() const { return port; }
		void startTransfer();

	private:
		int sockfd;
		Torrent_t const& torrent;
		unsigned int const port;

		class LeecherHandler_t;
		std::vector<LeecherHandler_t> leecherHandlers;
	};

	class Seeder_t::LeecherHandler_t {
	public:
		LeecherHandler_t(BT::Torrent_t const& t, Peer_t& seeder, Peer_t& leecher);
		void doTransfer(void);

	private:
		bool const communicatePortocolMessages();

		Torrent_t torrent;
		Peer_t seeder;
		Peer_t leecher;
	};
}
#endif
