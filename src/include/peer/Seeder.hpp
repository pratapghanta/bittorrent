#if !defined(SEEDER_HPP)
#define SEEDER_HPP

#include <memory>

#include "socket/ServerSocketObservable.hpp"
#include "socket/ServerSocketObserver.hpp"
#include "torrent/Torrent.hpp"

namespace BT 
{
	struct ConnectedSocketParcel;
	struct MessagingSocket;

	class Seeder : public IServerSocketObserver
	{
	public:
		Seeder(Torrent const& t, unsigned int const p);
		Seeder(Seeder const&) = delete;
		Seeder& operator=(Seeder const&) = delete;
		Seeder(Seeder&&) = default; // TODO
		Seeder& operator=(Seeder&&) = default; // TODO
		~Seeder();

		// Callbacks from IServerSocketObserver
		virtual void OnAcceptConnection(ConnectedSocketParcel const&);

	private:
		bool communicatePortocolMessages(MessagingSocket const&); 
		void startTransfer(MessagingSocket const&);

	private:
		std::unique_ptr<IServerSocketObservable> socket;
		Torrent torrent;
	};
}

#endif // !defined(SEEDER_HPP)
