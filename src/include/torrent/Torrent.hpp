#if !defined(TORRENT_HPP)
#define TORRENT_HPP

#include <vector>
#include <string>

#include "common/StatusCode.hpp"

namespace BT {

	using PieceHashList = std::vector<std::string>;
	
	class Torrent {
	public:
		Torrent() = default;
		Torrent(/* IN  */ std::string const& filename,
		        /* OUT */ STATUSCODE& rStatus);
		Torrent(Torrent const&) = default;
		Torrent& operator=(Torrent const&) = default;
		Torrent(Torrent&&) = default;
		Torrent& operator=(Torrent&&) = default;
		~Torrent() = default;

		void Reset();

	public:
		std::string name;
		std::string filename;
		unsigned int fileLength;
		unsigned int numOfPieces;
		unsigned int pieceLength;
		PieceHashList pieceHashes; 		    
		std::string infoHash;
	};

	std::ostream& operator<<(std::ostream& os, Torrent const& t);
}

#endif // !defined(TORRENT_HPP)
