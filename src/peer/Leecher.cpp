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
#include "peer/Message.hpp"
#include "peer/Peer.hpp"

namespace 
{
	std::string const getSaveFilenameForPiece(std::string const& saveFile, long const pieceIndex) 
	{
		std::stringstream ss;
		unsigned int const endPos = saveFile.rfind(BT::Defaults::torrentFileExtension);
		ss << saveFile.substr(0, endPos) << "_" << pieceIndex;

		return ss.str();
	}

	bool const isPieceAvailableAtSeeder(BT::Message_t const& msg, long const pieceIndex) 
	{
		auto piecesInfo = msg.getBitfield();
		return piecesInfo[pieceIndex] == 1;
	}

	bool isTransferSuccessful(BT::Torrent_t const& torrent, long const pieceIndex, std::string const& fileContents)
	{
		unsigned char hash[BT::Defaults::Sha1MdSize] = "";

		SHA1(reinterpret_cast<unsigned char const*>(fileContents.c_str()), fileContents.length(), hash);
		for (unsigned int i = 0; i < BT::Defaults::Sha1MdSize - 1; i++)
			if (hash[i] == '\0')
				hash[i] = '_';

		return torrent.GetPieceHashes()[pieceIndex].compare(reinterpret_cast<char*>(hash));
	}
}

BT::Leecher_t::Leecher_t(BT::Torrent_t const t, Peer_t const& s)
	: torrent(t), 
	  seeder(s) 
{
	leecher.EstablishConnectionTo(seeder);
}

bool const BT::Leecher_t::communicatePortocolMessages() 
{
	seeder.Send(BT::Defaults::handshakeMessage.c_str(), BT::Defaults::handshakeMessage.length());
	seeder.Send(torrent.GetInfoHash().c_str(), BT::Defaults::Sha1MdSize - 1);
	seeder.Send(leecher.GetId().c_str(), BT::Defaults::Sha1MdSize - 1);

	char buffer[BT::Defaults::MaxBufferSize] = "";
	seeder.Receive(buffer, BT::Defaults::handshakeMessage.length());
	if (BT::Defaults::handshakeMessage.compare(buffer) != 0)
		return false;

	auto inSameSwarm = [&]() 
	{
		memset(buffer, 0, BT::Defaults::MaxBufferSize);
		seeder.Receive(buffer, BT::Defaults::Sha1MdSize - 1);
		return torrent.GetInfoHash().compare(buffer) == 0;
	};

	auto expectedSeeder = [&]() 
	{
		memset(buffer, 0, BT::Defaults::MaxBufferSize);
		seeder.Receive(buffer, BT::Defaults::Sha1MdSize - 1);
		return seeder.GetId().compare(buffer) == 0;
	};

	return inSameSwarm() && expectedSeeder();
}

bool const BT::Leecher_t::getPieceFromSeeder(long const interestedPiece) 
{
	auto msg = seeder.ReceiveMessage(BT::Message_t::MessageType::BITFIELD);

	if (!isPieceAvailableAtSeeder(msg, interestedPiece)) 
	{
		seeder.SendMessage(Message_t::getNotInterestedMessage());
		return false;
	}

	seeder.SendMessage(Message_t::getInterestedMessage());
	msg = seeder.ReceiveMessage(BT::Message_t::MessageType::CHOKE); /* Expecting choke/unchoke */
	if (msg.isChoked()) 
		return false;

	int const begin = 0;
	auto requestDetails = BT::Request_t(interestedPiece, begin, torrent.GetPieceLength());
	seeder.SendMessage(Message_t::getRequestMessage(requestDetails));

	auto isEndOfPiece = [&](long const currPos) 
	{
		return currPos >= torrent.GetPieceLength();
	};
	
	auto isEndOfDataFile = [&](long const currPos) 
	{
		return (interestedPiece * torrent.GetPieceLength()) + begin + currPos >= torrent.GetFileLength();
	};

	std::string fileContents;
	while (!isEndOfPiece(fileContents.length()) && !isEndOfDataFile(fileContents.length())) 
	{
		if (fileContents.length() % BT::Defaults::BlockSize == 0)
		{
			auto pieceMsg = seeder.ReceiveMessage(BT::Message_t::MessageType::PIECE);
			if (!(pieceMsg.getPiece() == BT::Piece_t(interestedPiece, fileContents.length(), nullptr)))
				return false;
			seeder.SendMessage(BT::Message_t::getKeepAliveMessage());
		}

		char dataBuf[2] = "\0";
		seeder.Receive(dataBuf, 1);
		fileContents += std::string(dataBuf);
	}

	BT::CBinaryFileHandler fileHndl(getSaveFilenameForPiece(torrent.GetFileName(), interestedPiece));
	fileHndl.Put(fileContents);

	return isTransferSuccessful(torrent, interestedPiece, fileContents);
}

void BT::Leecher_t::startTransfer()
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
		seeder.SendMessage(Message_t::getNotInterestedMessage());
	}
}
