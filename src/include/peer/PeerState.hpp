#if !defined(PEER_STATE_HPP)
#define PEER_STATE_HPP

#include <string>
#include <vector>

namespace BT 
{
	struct PeerState 
	{
        bool bSeederIsChokingLeecher; // Leecher is choked
		bool bSeederIsInterested;
		bool bLeecherIsChokingSeeder; // Seeder is choked
		bool bLeecherIsInterested;

        PeerState()
            : bSeederIsChokingLeecher{true},
              bSeederIsInterested{false},
              bLeecherIsChokingSeeder{true},
              bLeecherIsInterested{false}
        {}
	};
}

#endif // !defined(PEER_STATE_HPP)
