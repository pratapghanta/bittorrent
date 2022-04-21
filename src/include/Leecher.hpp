#ifndef LEECHER_HPP
#define LEECHER_HPP

#include <vector>

#include "Peer.hpp"
#include "Config.hpp"
#include "Torrent.hpp"

namespace BT {
	class Leecher_t {
	public:
		Leecher_t(BT::Torrent_t const& t, Peer_t const& seeder);
		void startTransfer();

	private:
		bool const communicatePortocolMessages();
		bool const getPieceFromSeeder(long const interestedPiece);

		BT::Torrent_t const& torrent;
		BT::Peer_t seeder;
		BT::Peer_t leecher;
	};
}

#endif
