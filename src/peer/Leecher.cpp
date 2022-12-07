#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h> //ip hdeader library (must come before ip_icmp.h)
#include <netinet/ip_icmp.h> //icmp header
#include <arpa/inet.h> //internet address library
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <openssl/sha.h>

#include "common/Defaults.hpp"
#include "common/Helpers.hpp"
#include "peer/BinaryFileHandler.hpp"
#include "peer/Leecher.hpp"
#include "peer/MessageParcel.hpp"
#include "peer/MessageParcelFactory.hpp"
#include "peer/Peer.hpp"

namespace 
{
	std::string const getSaveFilenameForPiece(std::string const& saveFile, long const pieceIndex) 
	{
		std::stringstream ss;
		unsigned int const endPos = saveFile.rfind(BT::Defaults::TorrentFileExtension);
		ss << saveFile.substr(0, endPos) << "_" << pieceIndex;

		return ss.str();
	}

	bool const isPieceAvailableAtSeeder(BT::MessageParcel const& msg, long const pieceIndex) 
	{
		auto piecesInfo = msg.GetBitfield();
		return piecesInfo[pieceIndex] == 1;
	}

	bool isTransferSuccessful(BT::Torrent const& torrent, long const pieceIndex, std::string const& fileContents)
	{
		unsigned char hash[BT::Defaults::Sha1MdSize] = "";

		SHA1(reinterpret_cast<unsigned char const*>(fileContents.c_str()), fileContents.length(), hash);
		for (unsigned int i = 0; i < BT::Defaults::Sha1MdSize - 1; i++)
			if (hash[i] == '\0')
				hash[i] = '_';

		return torrent.pieceHashes[pieceIndex].compare(reinterpret_cast<char*>(hash));
	}
}

BT::Leecher::Leecher(BT::Torrent const t, Peer const& s)
	: torrent(t), 
	  seeder(s) 
{
	leecher.EstablishConnectionTo(seeder);
}

bool const BT::Leecher::communicatePortocolMessages() 
{
	seeder.Send(BT::Defaults::HandshakeMessage.c_str(), BT::Defaults::HandshakeMessage.length());
	seeder.Send(torrent.infoHash.c_str(), BT::Defaults::Sha1MdSize - 1);
	seeder.Send(leecher.GetId().c_str(), BT::Defaults::Sha1MdSize - 1);

	char buffer[BT::Defaults::MaxBufferSize] = "";
	seeder.Receive(buffer, BT::Defaults::HandshakeMessage.length());
	if (BT::Defaults::HandshakeMessage.compare(buffer) != 0)
		return false;

	auto inSameSwarm = [&]() 
	{
		memset(buffer, 0, BT::Defaults::MaxBufferSize);
		seeder.Receive(buffer, BT::Defaults::Sha1MdSize - 1);
		return torrent.infoHash.compare(buffer) == 0;
	};

	auto expectedSeeder = [&]() 
	{
		memset(buffer, 0, BT::Defaults::MaxBufferSize);
		seeder.Receive(buffer, BT::Defaults::Sha1MdSize - 1);
		return seeder.GetId().compare(buffer) == 0;
	};

	return inSameSwarm() && expectedSeeder();
}

bool const BT::Leecher::getPieceFromSeeder(long const interestedPiece) 
{
	auto msg = seeder.ReceiveMessage(MessageType::BITFIELD);

	if (!isPieceAvailableAtSeeder(msg, interestedPiece)) 
	{
		seeder.SendMessage(MessageParcelFactory::GetNotInterestedMessage());
		return false;
	}

	seeder.SendMessage(MessageParcelFactory::GetInterestedMessage());
	msg = seeder.ReceiveMessage(MessageType::CHOKE); /* Expecting choke/unchoke */
	if (msg.IsChoked()) 
		return false;

	int const begin = 0;
	auto requestDetails = BT::RequestParcel(interestedPiece, begin, torrent.pieceLength);
	seeder.SendMessage(MessageParcelFactory::GetRequestMessage(requestDetails));

	auto isEndOfPiece = [&](long const currPos) 
	{
		return currPos >= torrent.pieceLength;
	};
	
	auto isEndOfDataFile = [&](long const currPos) 
	{
		return (interestedPiece * torrent.pieceLength) + begin + currPos >= torrent.fileLength;
	};

	std::string fileContents;
	while (!isEndOfPiece(fileContents.length()) && !isEndOfDataFile(fileContents.length())) 
	{
		if (fileContents.length() % BT::Defaults::BlockSize == 0)
		{
			auto pieceMsg = seeder.ReceiveMessage(MessageType::PIECE);
			if (!(pieceMsg.GetPiece() == BT::PieceParcel(interestedPiece, fileContents.length(), nullptr)))
				return false;
			seeder.SendMessage(MessageParcelFactory::GetKeepAliveMessage());
		}

		char dataBuf[2] = "\0";
		seeder.Receive(dataBuf, 1);
		fileContents += std::string(dataBuf);
	}

	BT::CBinaryFileHandler fileHndl(getSaveFilenameForPiece(torrent.filename, interestedPiece));
	fileHndl.Put(fileContents);

	return isTransferSuccessful(torrent, interestedPiece, fileContents);
}

void BT::Leecher::startTransfer()
{
	if (!communicatePortocolMessages())
		return;

	long interestedPiece = 1;

	bool const isTransferred = getPieceFromSeeder(interestedPiece);
	if (isTransferred)
	{
		/* Broadcast to all other peers */
		/* Print to log about the downloaded piece */
		/* Synchronize threads such that this piece is not downloaded again */
		seeder.SendMessage(MessageParcelFactory::GetNotInterestedMessage());
	}
}
