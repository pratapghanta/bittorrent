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
#include "peer/MessagingSocket.hpp"
#include "socket/ConnectedSocketParcel.hpp"
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
		MessagingSocket socket (parcel);
		std::thread th([&]() { 
			startTransfer(socket); 
		});
		th.detach();
	}


	bool Seeder::communicatePortocolMessages(MessagingSocket const& socket) 
	{
		using namespace BT::Defaults;

		socket.Send(HandshakeMessage.c_str(), HandshakeMessage.length());
		socket.Send(torrent.infoHash.c_str(), Sha1MdSize);
		socket.Send(socket.GetFromId().c_str(), Sha1MdSize);

		char buffer[MaxBufferSize] = "";
		memset(buffer, 0, MaxBufferSize);
		socket.Receive(buffer, HandshakeMessage.length());

		if (std::string(buffer).compare(HandshakeMessage) != 0)
		{
			return false;
		}

		auto inSameSwarm = [&]() {
			memset(buffer, 0, MaxBufferSize);
			socket.Receive(buffer, Sha1MdSize);
			return torrent.infoHash.compare(buffer) == 0;
		};

		auto isExpectedHost = [&]() {
			memset(buffer, 0, MaxBufferSize);
			socket.Receive(buffer, Sha1MdSize);
			return socket.GetToId().compare(buffer) == 0;
		};

		return inSameSwarm() && isExpectedHost();
	}

	void Seeder::startTransfer(MessagingSocket const& socket) 
	{
		if (!communicatePortocolMessages(socket))
		{
			return;
		}

		long totalBytesTransferred = 0;

		std::string const avaliablePieces(torrent.numOfPieces, '1');
		socket.SendMessage(MessageParcelFactory::GetBitfieldMessage(avaliablePieces));

		auto msg = socket.ReceiveMessage(MessageType::INTERESTED);
		while (msg.IsInterested()) 
		{
			socket.SendMessage(MessageParcelFactory::GetUnChokedMessage());

			auto requestMsg = socket.ReceiveMessage(MessageType::REQUEST);
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
					socket.SendMessage(pieceMsg);

					auto keepAlive = socket.ReceiveMessage(MessageType::INTERESTED);
				}

				std::string const& data = fileHndl.Get();
				if (data.empty())
					break;
				socket.Send(data.c_str(), data.length());
				bytesTransfered += data.length();
			}

			totalBytesTransferred += bytesTransfered;

			msg = socket.ReceiveMessage(MessageType::INTERESTED);
			if (msg.IsKeepAlive()) usleep(1000000);
		}
	}
}
