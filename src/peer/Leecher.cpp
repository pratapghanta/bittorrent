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
#include "socket/IPv4Socket.hpp"

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

namespace BT
{
	Leecher::Leecher(Torrent const t, Peer const& s)
		: torrent(t), 
		  seeder(s) 
	{
		socket = IPv4ClientSocket::CreateTCPSocket();
		socket->Register(this);

		socket->ConnectToServer(seeder.ip, seeder.port);
	}

	void Leecher::OnConnect(ConnectedSocketParcel const& parcel)
	{
		// Leecher has connected socket. So, it can use it to transfer data
	}

	bool const Leecher::communicatePortocolMessages() 
	{
#if 0		
		seeder.Send(Defaults::HandshakeMessage.c_str(), Defaults::HandshakeMessage.length());
		seeder.Send(torrent.infoHash.c_str(), Defaults::Sha1MdSize - 1);
		seeder.Send(leecher.GetId().c_str(), Defaults::Sha1MdSize - 1);

		char buffer[Defaults::MaxBufferSize] = "";
		seeder.Receive(buffer, Defaults::HandshakeMessage.length());
		if (Defaults::HandshakeMessage.compare(buffer) != 0)
			return false;

		auto inSameSwarm = [&]() 
		{
			memset(buffer, 0, Defaults::MaxBufferSize);
			seeder.Receive(buffer, Defaults::Sha1MdSize - 1);
			return torrent.infoHash.compare(buffer) == 0;
		};

		auto expectedSeeder = [&]() 
		{
			memset(buffer, 0, Defaults::MaxBufferSize);
			seeder.Receive(buffer, Defaults::Sha1MdSize - 1);
			return seeder.GetId().compare(buffer) == 0;
		};

		return inSameSwarm() && expectedSeeder();
#endif
		return true;
	}

	bool const Leecher::getPieceFromSeeder(long const interestedPiece) 
	{
#if 0
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
		auto requestDetails = RequestParcel(interestedPiece, begin, torrent.pieceLength);
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
			if (fileContents.length() % Defaults::BlockSize == 0)
			{
				auto pieceMsg = seeder.ReceiveMessage(MessageType::PIECE);
				if (!(pieceMsg.GetPiece() == PieceParcel(interestedPiece, fileContents.length(), nullptr)))
					return false;
				seeder.SendMessage(MessageParcelFactory::GetKeepAliveMessage());
			}

			char dataBuf[2] = "\0";
			seeder.Receive(dataBuf, 1);
			fileContents += std::string(dataBuf);
		}

		CBinaryFileHandler fileHndl(getSaveFilenameForPiece(torrent.filename, interestedPiece));
		fileHndl.Put(fileContents);

		return isTransferSuccessful(torrent, interestedPiece, fileContents);
#endif
		return true;
	}

	void Leecher::startTransfer()
	{
#if 0
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
#endif
	}
}