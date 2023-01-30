#if !defined(LEECHER_HPP)
#define LEECHER_HPP

#include <memory>

#include "peer/ClientSocketObservable.hpp"
#include "peer/ClientSocketObserver.hpp"
#include "peer/Peer.hpp"
#include "torrent/Torrent.hpp"

namespace BT 
{
	struct ConnectedSocketParcel;
	class MessagingSocket;

	class Leecher : public IClientSocketObserver
	{
	public:
		Leecher(BT::Torrent const t, Peer const& seeder);
		virtual ~Leecher();

		virtual void OnConnect(ConnectedSocketParcel const&) override;

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
