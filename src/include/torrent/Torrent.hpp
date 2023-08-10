#if !defined(TORRENT_HPP)
#define TORRENT_HPP

#include <iostream>
#include <vector>
#include <string>

#include "common/StatusCode.hpp"

namespace BT 
{
	using PieceHashList = std::vector<std::string>;

	class Metainfo;	
	struct Torrent 
	{
	public:
		Torrent();
		Torrent(std::string const&, STATUSCODE&);
		~Torrent() = default;

		Torrent(Torrent const&) = default;
		Torrent& operator=(Torrent const&) = default;

		Torrent(Torrent&&) = default;
		Torrent& operator=(Torrent&&) = default;

		void Reset();

	private:
		STATUSCODE extractTorrentDetails(Metainfo&);

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
