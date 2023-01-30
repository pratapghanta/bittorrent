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
#include "peer/ConnectedSocketParcel.hpp"
#include "peer/BinaryFileHandler.hpp"
#include "peer/MessageParcel.hpp"
#include "peer/MessageParcelFactory.hpp"
#include "peer/MessagingSocket.hpp"
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
	Seeder::Seeder(Torrent const& t, 
	               unsigned int const port, 
				   unsigned int const maxConnections) 
		: torrent(t)
	{
		serverSocket = IPv4ServerSocket::CreateTCPSocket(this, port, maxConnections);
	}

	Seeder::~Seeder() 
	{
		serverSocket->Unregister(this);
	}

	void Seeder::OnAcceptConnection(ConnectedSocketParcel const& parcel)
	{
		MessagingSocket messagingSocket (parcel);
		transfer(messagingSocket); // TODO: Start a thread for n-n communication
	}

	bool Seeder::handshake(MessagingSocket const& messagingSocket) 
	{
		using namespace BT::Defaults;

		messagingSocket.Send(HandshakeMessage.c_str(), HandshakeMessage.length());
		messagingSocket.Send(torrent.infoHash.c_str(), Sha1MdSize);
		messagingSocket.Send(messagingSocket.GetToId().c_str(), Sha1MdSize);

		char buffer[MaxBufferSize] = "";
		memset(buffer, 0, MaxBufferSize);
		messagingSocket.Receive(buffer, HandshakeMessage.length());
		if (std::string(buffer).compare(HandshakeMessage) != 0)
		{
			return false;
		}

		auto inSameSwarm = [&]() {
			memset(buffer, 0, MaxBufferSize);
			messagingSocket.Receive(buffer, Sha1MdSize);
			return torrent.infoHash.compare(buffer) == 0;
		};

		auto isExpectedHost = [&]() {
			memset(buffer, 0, MaxBufferSize);
			messagingSocket.Receive(buffer, Sha1MdSize);
			return messagingSocket.GetToId().compare(buffer) == 0;
		};

		return inSameSwarm() && isExpectedHost();
	}

	void Seeder::messageLoop(MessagingSocket const& messagingSocket)
	{
		auto msg = messagingSocket.ReceiveMessage(); // MessageType::INTERESTED
		switch(msg.GetType())
		{
			case MessageType::CHOKE:
			{
				// Not implemented.
				break;
			}
			case MessageType::UNCHOKE:
			{

			}
			case MessageType::INTERESTED:
			{
				if (msg.IsKeepAlive())
				{

				}
				else
				{
					messagingSocket.SendMessage(MessageParcelFactory::GetUnChokedMessage());
				}
				messageLoop(messagingSocket);
				break;
			}
			case MessageType::NOTINTERESTED:
			{
				break;
			}
			case MessageType::HAVE:
			{

			}
			case MessageType::BITFIELD:
			{

			}
			case MessageType::REQUEST:
			{

			}
			case MessageType::PIECE:
			{

			}
			case MessageType::CANCEL:
			{
				// Not implemented.
			}
			default:
			{
				// throw exception
				break;
			}
		}
	}

	void Seeder::transfer(MessagingSocket const& messagingSocket) 
	{
		if (!handshake(messagingSocket))
		{
			return;
		}

		long totalBytesTransferred = 0;

		std::string const avaliablePieces(torrent.numOfPieces, '1');
		messagingSocket.SendMessage(MessageParcelFactory::GetBitfieldMessage(avaliablePieces));
		
		
		
		#if 0
		
		while (msg.IsInterested()) 
		{
			messagingSocket.SendMessage(MessageParcelFactory::GetUnChokedMessage());

			auto requestMsg = messagingSocket.ReceiveMessage(); // MessageType::REQUEST
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
					messagingSocket.SendMessage(pieceMsg);

					auto keepAlive = messagingSocket.ReceiveMessage(); // MessageType::INTERESTED
				}

				std::string const& data = fileHndl.Get();
				if (data.empty())
					break;
				messagingSocket.Send(data.c_str(), data.length());
				bytesTransfered += data.length();
			}

			totalBytesTransferred += bytesTransfered;

			msg = messagingSocket.ReceiveMessage(); // MessageType::INTERESTED
			if (msg.IsKeepAlive()) usleep(1000000);
		}

		#endif
	}
}
