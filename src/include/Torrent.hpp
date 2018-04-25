#ifndef TORRENT_HPP
#define TORRENT_HPP

#include <vector>
#include <string>

namespace BT {

	using PieceHashList_t = std::vector<std::string>;
	using ConstPieceHashList_t = const std::vector<std::string>;

	class Torrent_t {
	public:
		Torrent_t(const std::string& filename);
		
		std::string const&  getName() const { return name; }
		std::string const&  getFileName() const { return filename; }
		int const getFileLength() const { return fileLength; }
		int const getPieceLength() const { return pieceLength; }
		int const getNumOfPieces() const { return numOfPieces; }
		ConstPieceHashList_t getPieceHashes() const { return pieceHashes; }
		std::string const&  getInfoHash()  const { return infoHash; }

	private:
		std::string name;
		std::string filename;
		unsigned int fileLength;
		unsigned int numOfPieces;
		unsigned int pieceLength;
		PieceHashList_t pieceHashes; 		    
		std::string infoHash;
	};

	std::ostream& operator<<(std::ostream& os, BT::Torrent_t const& t);
}
#endif
