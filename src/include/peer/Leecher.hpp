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
		virtual ~Leecher();

		// Callbacks from IClientSocketObserver
		virtual void OnConnect(ConnectedSocketParcel const&);

	private:
		bool const handshake(MessagingSocket const&);
		void transfer(MessagingSocket const&);
		bool const getPieceFromSeeder(MessagingSocket const&, long const);

		std::unique_ptr<IClientSocketObservable> clientSocket;
		Torrent torrent;
		Peer seeder;
	};
}

#endif // !defined(LEECHER_HPP)
