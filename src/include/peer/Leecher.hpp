#if !defined(LEECHER_HPP)
#define LEECHER_HPP

#include <memory>

#include "peer/Peer.hpp"
#include "peer/MessagingSocket.hpp"
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
		
		// Callbacks from IClientSocketObserver
		virtual void OnConnect(ConnectedSocketParcel const&);

	private:
		bool const getPieceFromSeeder(MessagingSocket const&, long const);
		bool const communicatePortocolMessages(MessagingSocket const&);

		std::unique_ptr<IClientSocketObservable> socket;
		Torrent torrent;
		Peer seeder;
	};
}

#endif // !defined(LEECHER_HPP)
