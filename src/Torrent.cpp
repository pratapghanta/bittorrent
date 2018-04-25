#include <iostream>
#include <cmath>

#include "Defaults.hpp"
#include "helpers.hpp"
#include "Torrent.hpp"
#include "Metainfo.hpp"
#include "MinimalTorrentParser.hpp"

namespace BT {
	BT::Torrent_t::Torrent_t(std::string const& torrent) {
		TorrentParser const &TP = MinimalTorrentParser();
		Metainfo const& mi = TP.doParse(torrent);

		MI_DictPtr_t const& data = mi.getData();
		MI_DictPtr_t const& infoDict = (*data)["info"].value.mDict;

		filename = torrent;
		name = std::string((*infoDict)["name"].value.cStr);
		fileLength = (*infoDict)["length"].value.nInt;
		pieceLength = (*infoDict)["piece length"].value.nInt;
		numOfPieces = static_cast<unsigned int>((fileLength <= pieceLength) ? 1 : ceil(fileLength * 1.0 / pieceLength));

		std::string const& hashesOfAllPieces = (*infoDict)["pieces"].value.cStr;
		if (numOfPieces != (hashesOfAllPieces.length() / BT::Defaults::Sha1MdSize))
			throwErrorAndExit("Invalid .torrent file.");
		
		for (unsigned int i = 0; i < numOfPieces; i++) {
			std::string const& ithPieceHash = hashesOfAllPieces.substr(i*BT::Defaults::Sha1MdSize, BT::Defaults::Sha1MdSize);
			pieceHashes.push_back(ithPieceHash);
		}
	}

	std::ostream& operator<<(std::ostream& os, BT::Torrent_t const& t) {
		os << "*** Torrent details ***" << std::endl;
		os << "    Name: " << t.getName() << std::endl;
		os << "    File size:" << t.getFileLength() << " bytes" << std::endl;
		os << "    Number of pieces: " << t.getNumOfPieces() << std::endl;
		os << "    Piece length: " << t.getPieceLength() << " bytes" << std::endl;
		os << "    Piece hashes:" << std::endl;
		for (auto itr = t.getPieceHashes().begin(); itr != t.getPieceHashes().end(); ++itr)
			os << "    " << *itr << std::endl;

		return os;
	}
}
