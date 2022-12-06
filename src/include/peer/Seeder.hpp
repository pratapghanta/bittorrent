#if !defined(SEEDER_HPP)
#define SEEDER_HPP

#include <vector>

#include "peer/Peer.hpp"
#include "torrent/Torrent.hpp"
#include "StartParams.hpp"

namespace BT {
	class Seeder_t {
	public:
		Seeder_t(Torrent const& t, unsigned int const p);

		Seeder_t(Seeder_t const&) = delete;
		Seeder_t& operator=(Seeder_t const&) = delete;

		Seeder_t(Seeder_t&&);
		Seeder_t& operator=(Seeder_t&&);

		~Seeder_t();

		unsigned int const GetPort() const { return port; }
		void StartTransfer();

	private:
		int sockfd;
		Torrent torrent;
		unsigned int port;

		class LeecherHandler_t;
		std::vector<LeecherHandler_t> leecherHandlers;
	};

	class Seeder_t::LeecherHandler_t {
	public:
		LeecherHandler_t(BT::Torrent const& t, Peer& seeder, Peer& leecher);
		void StartTransfer();

	private:
		bool const communicatePortocolMessages();

		Torrent torrent;
		Peer seeder;
		Peer leecher;
	};
}

#endif // !defined(SEEDER_HPP)
