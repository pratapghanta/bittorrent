#if !defined(LEECHER_HPP)
#define LEECHER_HPP

#include <memory>

#include "peer/Peer.hpp"
#include "socket/ClientSocketObservable.hpp"
#include "socket/ClientSocketObserver.hpp"
#include "torrent/Torrent.hpp"

namespace BT 
{
	struct ConnectedSocketParcel;
	class Leecher : public IClientSocketObserver
	{
	public:
		Leecher(BT::Torrent const t, Peer const& seeder);
		void startTransfer();

		// Callbacks from IClientSocketObserver
		virtual void OnConnect(ConnectedSocketParcel const&);

	private:
		bool const communicatePortocolMessages();
		bool const getPieceFromSeeder(long const interestedPiece);

		std::unique_ptr<IClientSocketObservable> socket;
		BT::Torrent const& torrent;
		BT::Peer seeder;
		BT::Peer leecher;
	};
}

#endif // !defined(LEECHER_HPP)
