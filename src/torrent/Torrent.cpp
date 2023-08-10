#include <iostream>
#include <cmath>

#include "common/Defaults.hpp"
#include "torrent/Metainfo.hpp"
#include "torrent/MinimalTorrentParser.hpp"
#include "torrent/Torrent.hpp"

namespace BT 
{
	Torrent::Torrent()
			: fileLength(0),
			  numOfPieces(0),
			  pieceLength(0) {}


	Torrent::Torrent(std::string const& torrent,
	                 STATUSCODE& status)
			: fileLength(0),
			  numOfPieces(0),
			  pieceLength(0) 
	{
		status = STATUSCODE::SC_SUCCESS;

		Metainfo mi;
		status = MinimalTorrentParser().Parse(torrent, mi);
		if (SC_FAILED(status))
		{
			return;
		}

		filename = torrent;
		status = extractTorrentDetails(mi);
		if (SC_FAILED(status))
		{
			return;
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
		if (t.numOfPieces == 0)
		{
			os << "*** Invalid torrent ***" << std::endl;
			return os;
		}

		os << "*** Torrent details ***" << std::endl;
		os << "    Name: " << t.name << std::endl;
		os << "    File size:" << t.fileLength << " bytes" << std::endl;
		os << "    Number of pieces: " << t.numOfPieces << std::endl;
		os << "    Piece length: " << t.pieceLength << " bytes" << std::endl;
		os << "    Piece hashes:" << std::endl;
		for (auto const& pieceHash : t.pieceHashes)
		{
			os << "        " << pieceHash << std::endl;
		}

		return os;
	}

	STATUSCODE Torrent::extractTorrentDetails(Metainfo& mi)
	{
		using Defaults::Sha1MdSize;

		MI_DictPtr const infoDict = mi.mData["info"].GetDictPtr();
		pieceLength = (*infoDict)["piece length"].GetInt();
		if (pieceLength == 0) 
		{
			Reset();
			return STATUSCODE::SC_FAIL_BAD_TORRENT;
		}
		
		std::string const& hashesOfAllPieces = (*infoDict)["pieces"].GetString();
		if (0 != (hashesOfAllPieces.length() % Sha1MdSize)) 
		{
			Reset();
			return STATUSCODE::SC_FAIL_BAD_TORRENT;
		}

		fileLength = (*infoDict)["length"].GetInt();
		numOfPieces = static_cast<unsigned int>((fileLength <= pieceLength) ? 1 : ceil(fileLength * 1.0 / pieceLength));
		if (0 == numOfPieces || numOfPieces != (hashesOfAllPieces.length() / Sha1MdSize)) 
		{
			Reset();
			return STATUSCODE::SC_FAIL_BAD_TORRENT;;
		}
		
		for (unsigned int i = 0; i < numOfPieces; i++) 
		{
			std::string const& ithPieceHash = hashesOfAllPieces.substr(i*BT::Defaults::Sha1MdSize, BT::Defaults::Sha1MdSize);
			pieceHashes.push_back(ithPieceHash);
		}

		name = (*infoDict)["name"].GetString();
		infoHash = mi.mInfoHash;
	}
}
