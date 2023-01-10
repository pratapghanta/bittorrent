#include <iostream>
#include <cmath>

#include "common/Defaults.hpp"
#include "torrent/Metainfo.hpp"
#include "torrent/MinimalTorrentParser.hpp"
#include "torrent/Torrent.hpp"

namespace BT 
{
	Torrent::Torrent(/* IN  */ std::string const& torrent,
	                 /* OUT */ STATUSCODE& rStatus)
			: fileLength(0),
			  numOfPieces(0),
			  pieceLength(0) 
	{
		rStatus = STATUSCODE::SC_SUCCESS;

		Metainfo_t mi;
		rStatus = MinimalTorrentParser_t().Parse(torrent, mi);
		if (SC_FAILED(rStatus))
		{
			return;
		}

		MI_DictPtr_t const infoDict = mi.mData["info"].GetDictPtr();
		filename = torrent;
		name = (*infoDict)["name"].GetString();
		fileLength = (*infoDict)["length"].GetInt();
		pieceLength = (*infoDict)["piece length"].GetInt();
		numOfPieces = static_cast<unsigned int>((fileLength <= pieceLength) ? 
												 1 : ceil(fileLength * 1.0 / pieceLength));

		std::string const& hashesOfAllPieces = (*infoDict)["pieces"].GetString();
		if (numOfPieces != (hashesOfAllPieces.length() / BT::Defaults::Sha1MdSize)) 
		{
			rStatus = STATUSCODE::SC_FAIL_BAD_TORRENT;
			Reset();
			return;
		}
		
		for (unsigned int i = 0; i < numOfPieces; i++) 
		{
			std::string const& ithPieceHash = hashesOfAllPieces.substr(i*BT::Defaults::Sha1MdSize, BT::Defaults::Sha1MdSize);
			pieceHashes.push_back(ithPieceHash);
		}
	}

	void Torrent::Reset() 
	{
		fileLength = 0;
		numOfPieces = 0;
		pieceLength = 0;
		pieceHashes.clear();
		infoHash.clear();
	}

	std::ostream& operator<<(std::ostream& os, BT::Torrent const& t) 
	{
		os << "*** Torrent details ***" << std::endl;
		os << "    Name: " << t.name << std::endl;
		os << "    File size:" << t.fileLength << " bytes" << std::endl;
		os << "    Number of pieces: " << t.numOfPieces << std::endl;
		os << "    Piece length: " << t.pieceLength << " bytes" << std::endl;
		os << "    Piece hashes:" << std::endl;
		for (auto itr = t.pieceHashes.begin(); itr != t.pieceHashes.end(); ++itr)
			os << "    " << *itr << std::endl;

		return os;
	}
}
