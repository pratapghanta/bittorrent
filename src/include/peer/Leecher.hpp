#if !defined(LEECHER_HPP)
#define LEECHER_HPP

#include <vector>

#include "peer/Peer.hpp"
#include "torrent/Torrent.hpp"
#include "StartParams.hpp"

namespace BT {
	class Leecher {
	public:
		Leecher(BT::Torrent const t, Peer const& seeder);
		void startTransfer();

	private:
		bool const communicatePortocolMessages();
		bool const getPieceFromSeeder(long const interestedPiece);

		BT::Torrent const& torrent;
		BT::Peer seeder;
		BT::Peer leecher;
	};
}

#endif // !defined(LEECHER_HPP)
