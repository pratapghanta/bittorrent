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
#include <unordered_map>
#include <functional>

#include "common/Defaults.hpp"
#include "common/Helpers.hpp"
#include "common/Logger.hpp"
#include "peer/ConnectedSocketParcel.hpp"
#include "peer/BinaryFileHandler.hpp"
#include "peer/MessageParcel.hpp"
#include "peer/MessageParcelFactory.hpp"
#include "peer/MessagingSocket.hpp"
#include "peer/PeerState.hpp"
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
		if (serverSocket != nullptr)
		{
			serverSocket->Unregister(this);
		}
	}

	void Seeder::OnAcceptConnection(ConnectedSocketParcel const& parcel)
	{
		Trace("Accepted connection: (Server) [sockfd: %d] <==> %s:%u (client).", parcel.connectedSockfd, parcel.toIp, parcel.toPort);
		MessagingSocket messagingSocket(parcel);
		transfer(messagingSocket); // TODO: Start a thread for n-n communication
	}

	bool Seeder::handshake(MessagingSocket const& messagingSocket) 
	{
		using namespace BT::Defaults;

		messagingSocket.Send(HandshakeMessage.c_str(), HandshakeMessage.length());
		messagingSocket.Send(torrent.infoHash.c_str(), Sha1MdSize);
		messagingSocket.Send(messagingSocket.GetToId().c_str(), Sha1MdSize);

		CharBuffer buffer {};
		messagingSocket.Receive(&(buffer[0]), HandshakeMessage.length());
		if (HandshakeMessage.compare(&(buffer[0])) != 0)
		{
			return false;
		}

		auto inSameSwarm = [&]() {
			buffer.fill(0);
			messagingSocket.Receive(&(buffer[0]), Sha1MdSize);
			return torrent.infoHash.compare(&(buffer[0])) == 0;
		};

		auto isExpectedHost = [&]() {
			buffer.fill(0);
			messagingSocket.Receive(&(buffer[0]), Sha1MdSize);
			return messagingSocket.GetToId().compare(&(buffer[0])) == 0;
		};

		return inSameSwarm() && isExpectedHost();
	}

	void Seeder::messageLoop(MessagingSocket const& messagingSocket)
	{
		PeerState state;
		bool bTransferInProgress = true;
		MessageParcel msg;

		auto OnChokeMessageReceived = [&](){
			state.bSeederIsChokingLeecher = true;
			// Pausing transfer is NOT implemented.
		};

		auto OnUnchokeMessageReceived = [&](){
			state.bSeederIsChokingLeecher = false;
			std::string bitfield(torrent.numOfPieces, '1');
			messagingSocket.SendMessage(MessageParcelFactory::GetBitfieldMessage(bitfield));
		};

		auto OnInterestedMessageReceived = [&](){
			state.bLeecherIsInterested = true;	
			messagingSocket.SendMessage(MessageParcelFactory::GetUnChokedMessage());
			messagingSocket.SendMessage(MessageParcelFactory::GetInterestedMessage());
			state.bLeecherIsChokingSeeder = false;
			state.bSeederIsInterested = true;
		};

		auto OnNotInterestedMessageReceived = [&](){
			state.bLeecherIsInterested = false;
			bTransferInProgress = false;
		};

		auto OnRequestMessageReceived = [&](){
			// TODO: Different thread to handle data transfer?
			RequestParcel requestParcel = msg.GetRequest();
			
			CBinaryFileHandler fileHndl(getDataFilename(torrent.filename));
			uint32_t totalBytesRemaining = requestParcel.length;
			uint32_t bytesTransferred = 0;
			while (bytesTransferred < totalBytesRemaining)
			{
				unsigned int pieceSize = std::min(Defaults::BlockSize, totalBytesRemaining);

				PieceParcel pieceParcel;
				pieceParcel.index = requestParcel.index;
				pieceParcel.begin = bytesTransferred+1;
				pieceParcel.piece = new char[pieceSize+1];

				fileHndl.Seek((requestParcel.index * requestParcel.length) + bytesTransferred);
				unsigned int bytesRead = 0;
				fileHndl.Get(pieceSize, pieceParcel.piece, bytesRead);
				pieceParcel.piece[bytesRead] = '\0';
				bytesTransferred += bytesRead;

				messagingSocket.SendMessage(MessageParcelFactory::GetPieceMessage(pieceParcel));
			}

		};

		auto OnKeepaliveMessageReceived = [&](){
			
		};

		auto NotImplemented = [&](){};
		auto UnExpected = [&](){};

		static std::unordered_map<MessageType, std::function<void()>> messageHandlers;
		if (messageHandlers.empty())
		{
			messageHandlers[MessageType::CHOKE] = OnChokeMessageReceived;
			messageHandlers[MessageType::UNCHOKE] = OnUnchokeMessageReceived;
			messageHandlers[MessageType::INTERESTED] = OnInterestedMessageReceived;
			messageHandlers[MessageType::NOTINTERESTED] = OnNotInterestedMessageReceived;
			messageHandlers[MessageType::HAVE] = NotImplemented;
			messageHandlers[MessageType::BITFIELD] = UnExpected;
			messageHandlers[MessageType::REQUEST] = OnRequestMessageReceived;
			messageHandlers[MessageType::PIECE] = UnExpected;
			messageHandlers[MessageType::CANCEL] = NotImplemented;
		}

		while (bTransferInProgress)
		{
			msg = messagingSocket.ReceiveMessage();
			if (msg.IsKeepAlive())
			{
				OnKeepaliveMessageReceived();
			}
			else
			{
				auto itr = messageHandlers.find(msg.GetType());
				if (itr != messageHandlers.end())
				{
					itr->second();
				}
			}
		}
	}

	void Seeder::transfer(MessagingSocket const& messagingSocket) 
	{
		if (!handshake(messagingSocket))
		{
			return;
		}

		messageLoop(messagingSocket);
	}
}
