#include <iostream>
#include <cmath>

#include "Torrent.hpp"
#include "Defaults.hpp"
#include "Metainfo.hpp"
#include "MinimalTorrentParser.hpp"

namespace BT {
	Torrent_t::Torrent_t(/* IN */  std::string const& torrent,
	                     /* OUT */ STATUSCODE& rStatus)
			: mFileLength(0), mNumOfPieces(0), mPieceLength(0) {
		Metainfo_t mi;
		MinimalTorrentParser_t().Parse(torrent, mi);
		MI_DictPtr_t const& infoDict = (mi.mData)["info"].value.mDict;

		mFilename = torrent;
		mName = std::string((*infoDict)["name"].value.cStr);
		mFileLength = (*infoDict)["length"].value.nInt;
		mPieceLength = (*infoDict)["piece length"].value.nInt;
		mNumOfPieces = static_cast<unsigned int>((mFileLength <= mPieceLength) ? 
													1 :
													ceil(mFileLength * 1.0 / mPieceLength));

		std::string const& hashesOfAllPieces = (*infoDict)["pieces"].value.cStr;
		if (mNumOfPieces != (hashesOfAllPieces.length() / BT::Defaults::Sha1MdSize)) {
			rStatus = STATUSCODE::SC_FAIL_BAD_TORRENT;
			Reset();
			return;
		}
		
		for (unsigned int i = 0; i < mNumOfPieces; i++) {
			std::string const& ithPieceHash = hashesOfAllPieces.substr(i*BT::Defaults::Sha1MdSize, BT::Defaults::Sha1MdSize);
			mPieceHashes.push_back(ithPieceHash);
		}
	}

	void Torrent_t::Reset() {
		mFileLength = 0;
		mNumOfPieces = 0;
		mPieceLength = 0;
		mPieceHashes.clear();
		mInfoHash.clear();
	}

	std::ostream& operator<<(std::ostream& os, BT::Torrent_t const& t) {
		os << "*** Torrent details ***" << std::endl;
		os << "    Name: " << t.GetName() << std::endl;
		os << "    File size:" << t.GetFileLength() << " bytes" << std::endl;
		os << "    Number of pieces: " << t.GetNumOfPieces() << std::endl;
		os << "    Piece length: " << t.GetPieceLength() << " bytes" << std::endl;
		os << "    Piece hashes:" << std::endl;
		for (auto itr = t.GetPieceHashes().begin(); itr != t.GetPieceHashes().end(); ++itr)
			os << "    " << *itr << std::endl;

		return os;
	}
}
