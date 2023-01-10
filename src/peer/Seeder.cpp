#include "peer/Seeder.hpp"

#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <thread>
#include <map>
#include <memory>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <openssl/sha.h>

#include "common/Defaults.hpp"
#include "common/Helpers.hpp"
#include "common/Logger.hpp"
#include "peer/BinaryFileHandler.hpp"
#include "peer/MessageParcel.hpp"
#include "peer/MessageParcelFactory.hpp"
#include "socket/ConnectedSocketParcel.hpp"
#include "socket/ConnectedSocket.hpp"
#include "socket/IPv4Socket.hpp"

namespace 
{
	std::string const getDataFilename(std::string const& torrentFilename) 
	{
		unsigned int endPos = torrentFilename.rfind(BT::Defaults::TorrentFileExtension);
		return torrentFilename.substr(0, endPos);
	}
}

namespace BT 
{
	Seeder::Seeder(Torrent const& t, unsigned int const port) 
		: torrent(t)
	{
		socket = IPv4ServerSocket::CreateTCPSocket(port);
		socket->Register(this);
		socket->AcceptConnection();
	}

	Seeder::~Seeder() 
	{
	}

	void Seeder::OnAcceptConnection(ConnectedSocketParcel const& parcel)
	{
		ConnectedSocket connectedSocket (parcel);
		#if 0
		
		LeecherHandler lh(torrent, seeder, leecher);
		leecherHandlers.push_back(std::move(lh));
		
		std::thread sth([&]() { leecherHandlers[0].StartTransfer(); });
		#endif
	}


	bool const Seeder::communicatePortocolMessages(void) 
	{
		#if 0
		bool const handshakeFailed = false;

		leecher.Send(Defaults::HandshakeMessage.c_str(), Defaults::HandshakeMessage.length());
		leecher.Send(torrent.infoHash.c_str(), Defaults::Sha1MdSize - 1);
		leecher.Send(seeder.GetId().c_str(), Defaults::Sha1MdSize - 1);

		char buffer[Defaults::MaxBufferSize] = "";
		memset(buffer, 0, Defaults::MaxBufferSize);
		leecher.Receive(buffer, Defaults::HandshakeMessage.length());

		if (std::string(buffer).compare(Defaults::HandshakeMessage) != 0)
			return handshakeFailed;

		auto inSameSwarm = [&]() {
			memset(buffer, 0, Defaults::MaxBufferSize);
			leecher.Receive(buffer, Defaults::Sha1MdSize - 1);
			return torrent.infoHash.compare(buffer) == 0;
		};

		auto isExpectedHost = [&]() {
			memset(buffer, 0, Defaults::MaxBufferSize);
			leecher.Receive(buffer, Defaults::Sha1MdSize - 1);
			return leecher.GetId().compare(buffer) == 0;
		};

		return inSameSwarm() && isExpectedHost();
		#endif
		return true;
	}

	void Seeder::StartTransfer(void) 
	{
		if (!communicatePortocolMessages())
			return;

		long totalBytesTransferred = 0;

		std::string const avaliablePieces(torrent.numOfPieces, '1');
		leecher.SendMessage(MessageParcelFactory::GetBitfieldMessage(avaliablePieces));

		auto msg = leecher.ReceiveMessage(MessageType::INTERESTED);
		while (msg.IsInterested()) {
			leecher.SendMessage(MessageParcelFactory::GetUnChokedMessage());

			auto requestMsg = leecher.ReceiveMessage(MessageType::REQUEST);
			RequestParcel const request = requestMsg.GetRequest();

			CBinaryFileHandler fileHndl(getDataFilename(torrent.filename));
			fileHndl.Seek((request.index * request.length) + request.begin);

			long block = 0;
			long bytesTransfered = 0;
			while (bytesTransfered < request.length)
			{
				if (bytesTransfered % (Defaults::BlockSize) == 0)
				{
					block++;

					PieceParcel piece(request.index, bytesTransfered + 1, nullptr);
					MessageParcel const& pieceMsg = MessageParcelFactory::GetPieceMessage(piece);
					leecher.SendMessage(pieceMsg);

					auto keepAlive = leecher.ReceiveMessage(MessageType::INTERESTED);
				}

				std::string const& data = fileHndl.Get();
				if (data.empty())
					break;
				leecher.Send(data.c_str(), data.length());
				bytesTransfered += data.length();
			}

			totalBytesTransferred += bytesTransfered;

			msg = leecher.ReceiveMessage(MessageType::INTERESTED);
			if (msg.IsKeepAlive()) usleep(1000000);
		}
	}
}
