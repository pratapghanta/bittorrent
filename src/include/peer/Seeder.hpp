#if !defined(SEEDER_HPP)
#define SEEDER_HPP

#include <memory>

#include "peer/ServerSocketObservable.hpp"
#include "peer/ServerSocketObserver.hpp"
#include "torrent/Torrent.hpp"

namespace BT 
{
	struct ConnectedSocketParcel;
	struct MessagingSocket;

	class Seeder : public IServerSocketObserver
	{
	public:
		Seeder(Torrent const&, unsigned int const, unsigned int const);
		Seeder(Seeder const&) = delete;
		Seeder& operator=(Seeder const&) = delete;
		Seeder(Seeder&&) = default; // TODO
		Seeder& operator=(Seeder&&) = default; // TODO
		virtual ~Seeder();

		virtual void OnAcceptConnection(ConnectedSocketParcel const&);

	private:
		bool handshake(MessagingSocket const&); 
		void transfer(MessagingSocket const&);
		void messageLoop(MessagingSocket const&);

	private:
		std::unique_ptr<IServerSocketObservable> serverSocket;
		Torrent torrent;
	};
}

#endif // !defined(SEEDER_HPP)
