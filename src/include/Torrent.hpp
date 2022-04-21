#ifndef TORRENT_HPP
#define TORRENT_HPP

#include <vector>
#include <string>

#include "Errors.hpp"

namespace BT {

	using PieceHashList_t = std::vector<std::string>;
	
	class Torrent_t {
	public:
		Torrent_t(/* IN */  std::string const& filename,
		          /* OUT */ STATUSCODE& rStatus);
		
		void Reset();

		std::string const&  GetName() const { return mName; }
		std::string const&  GetFileName() const { return mFilename; }
		int const GetFileLength() const { return mFileLength; }
		int const GetPieceLength() const { return mPieceLength; }
		int const GetNumOfPieces() const { return mNumOfPieces; }
		const PieceHashList_t GetPieceHashes() const { return mPieceHashes; }
		std::string const&  GetInfoHash()  const { return mInfoHash; }

	private:
		std::string mName;
		std::string mFilename;
		unsigned int mFileLength;
		unsigned int mNumOfPieces;
		unsigned int mPieceLength;
		PieceHashList_t mPieceHashes; 		    
		std::string mInfoHash;
	};

	std::ostream& operator<<(std::ostream& os, Torrent_t const& t);
}
#endif
