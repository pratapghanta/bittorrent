#if !defined(SEEDER_HPP)
#define SEEDER_HPP

#include <vector>

#include "peer/Peer.hpp"
#include "torrent/Torrent.hpp"
#include "StartParams.hpp"

namespace BT {
	class Seeder {
	public:
		Seeder(Torrent const& t, unsigned int const p);

		Seeder(Seeder const&) = delete;
		Seeder& operator=(Seeder const&) = delete;

		Seeder(Seeder&&);
		Seeder& operator=(Seeder&&);

		~Seeder();

		unsigned int const GetPort() const { return port; }
		void StartTransfer();

	private:
		int sockfd;
		Torrent torrent;
		unsigned int port;

		class LeecherHandler;
		std::vector<LeecherHandler> leecherHandlers;
	};

	class Seeder::LeecherHandler {
	public:
		LeecherHandler(BT::Torrent const& t, Peer& seeder, Peer& leecher);
		void StartTransfer();

	private:
		bool const communicatePortocolMessages();

		Torrent torrent;
		Peer seeder;
		Peer leecher;
	};
}

#endif // !defined(SEEDER_HPP)
