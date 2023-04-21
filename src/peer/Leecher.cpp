#include <iostream>
#include <array>
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
#include <unordered_map>
#include <functional>

#include "common/Defaults.hpp"
#include "common/Helpers.hpp"
#include "peer/BinaryFileHandler.hpp"
#include "peer/Leecher.hpp"
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

	std::string const getSaveFilenameForPiece(std::string const& saveFile, long const pieceIndex) 
	{
		std::stringstream ss;
		unsigned int const endPos = saveFile.rfind(BT::Defaults::TorrentFileExtension);
		ss << saveFile.substr(0, endPos) << "_" << pieceIndex;
		return ss.str();
	}

	bool const allPiecesAvailableAtSeeder(BT::MessageParcel const& msg, BT::Torrent const& torrent) 
	{
		return (msg.GetBitfield() == std::string(torrent.numOfPieces, '1'));
	}

	bool isTransferSuccessful(BT::Torrent const& torrent, long const pieceIndex, std::string const& fileContents)
	{
		unsigned char hash[BT::Defaults::Sha1MdSize+1] = "";
		SHA1(reinterpret_cast<unsigned char const*>(fileContents.c_str()), fileContents.length(), hash);
		for (unsigned int i = 0; i < BT::Defaults::Sha1MdSize; i++)
		{
			if (hash[i] == '\0')
			{
				hash[i] = '_';
			}
		}
		return torrent.pieceHashes[pieceIndex].compare(reinterpret_cast<char*>(hash));
	}
}

namespace BT
{
	Leecher::Leecher(Torrent const t, Peer const& s)
		: torrent(t), 
		  seeder(s) 
	{
		clientSocket = IPv4ClientSocket::CreateTCPSocket(this, seeder.ip, seeder.port);
	}

	Leecher::~Leecher()
	{
		clientSocket->Unregister(this);
	}

	void Leecher::OnConnect(ConnectedSocketParcel const& parcel)
	{
		MessagingSocket messagingSocket (parcel);
		if (!handshake(messagingSocket))
		{
			return;
		}

		long interestedPiece = 0;
		getPieceFromSeeder(messagingSocket, interestedPiece);
		
		bool bTransferComplete = true;
		if (bTransferComplete)
		{
			/* Broadcast to all other peers */
			/* Print to log about the downloaded piece */
			/* Synchronize threads such that this piece is not downloaded again */
			messagingSocket.SendMessage(MessageParcelFactory::GetNotInterestedMessage());
		}
	}

	bool const Leecher::handshake(MessagingSocket const& messagingSocket) 
	{
		using namespace BT::Defaults;

		CharBuffer buffer;
		buffer.fill(0);
		messagingSocket.Receive(&(buffer[0]), HandshakeMessage.length());
		if (HandshakeMessage.compare(&(buffer[0])) != 0)
		{
			return false;
		}

		auto inSameSwarm = [&]() 
		{
			buffer.fill(0);
			messagingSocket.Receive(&(buffer[0]), Sha1MdSize);
			return torrent.infoHash.compare(&(buffer[0])) == 0;
		};

		auto expectedSeeder = [&]() 
		{
			buffer.fill(0);
			messagingSocket.Receive(&(buffer[0]), Defaults::Sha1MdSize);
			return messagingSocket.GetFromId().compare(&(buffer[0])) == 0;
		};

		if (inSameSwarm() && expectedSeeder())
		{
			messagingSocket.Send(HandshakeMessage.c_str(), HandshakeMessage.length());
			messagingSocket.Send(torrent.infoHash.c_str(), Sha1MdSize);
			messagingSocket.Send(messagingSocket.GetFromId().c_str(), Sha1MdSize);
			return true;
		}

		return false;
	}

#if 0
	bool const Leecher::getPieceFromSeeder(MessagingSocket const& messagingSocket, long const interestedPiece) 
	{
		auto msg = messagingSocket.ReceiveMessage(); // MessageType::BITFIELD

		
		messagingSocket.SendMessage(MessageParcelFactory::GetInterestedMessage());
		msg = messagingSocket.ReceiveMessage(); // MessageType::UNCHOKE /* Expecting choke/unchoke */
		if (msg.IsChoked())
		{
			return false;
		}

		int const begin = 0;
		auto requestDetails = RequestParcel(interestedPiece, begin, torrent.pieceLength);
		messagingSocket.SendMessage(MessageParcelFactory::GetRequestMessage(requestDetails));

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
				auto pieceMsg = messagingSocket.ReceiveMessage(); // MessageType::PIECE
				if (!(pieceMsg.GetPiece() == PieceParcel(interestedPiece, fileContents.length(), nullptr)))
				{
					return false;
				}
				messagingSocket.SendMessage(MessageParcelFactory::GetKeepAliveMessage());
			}

			char dataBuf[2] = "\0";
			messagingSocket.Receive(dataBuf, 1);
			fileContents += std::string(dataBuf);
		}

		CBinaryFileHandler fileHndl(getSaveFilenameForPiece(torrent.filename, interestedPiece));
		fileHndl.Put(fileContents);

		return isTransferSuccessful(torrent, interestedPiece, fileContents);

		return true;
	}
#endif

	void Leecher::getPieceFromSeeder(MessagingSocket const& messagingSocket,
	                                 long const piece)
	{
		PeerState state;
		bool bTransferInProgress = true;
		MessageParcel msg;
		uint32_t bytesRequested = torrent.pieceLength;
		uint32_t bytesTransferred = 0;
		
		CBinaryFileHandler fileHndl(getDataFilename(torrent.filename));

		auto OnChokeMessageReceived = [&](){
			state.bLeecherIsChokingSeeder = true;
			// Pausing transfer is NOT implemented.
		};

		auto OnUnchokeMessageReceived = [&](){
			state.bLeecherIsChokingSeeder = false;
		};

		auto OnInterestedMessageReceived = [&](){
			state.bSeederIsInterested = true;

			RequestParcel requestParcel;
			requestParcel.begin = 0;
			requestParcel.index = piece;
			requestParcel.length = torrent.pieceLength;
			messagingSocket.SendMessage(MessageParcelFactory::GetRequestMessage(requestParcel));
		};

		auto OnNotInterestedMessageReceived = [&](){
			state.bSeederIsInterested = false;
			bTransferInProgress = false;
		};

		auto OnBitfieldMessageReceived = [&](){
			if (allPiecesAvailableAtSeeder(msg, torrent)) 
			{
				messagingSocket.SendMessage(MessageParcelFactory::GetInterestedMessage());
				state.bLeecherIsInterested = true;
			}
			else
			{
				state.bLeecherIsInterested = false;
				bTransferInProgress = false;
				messagingSocket.SendMessage(MessageParcelFactory::GetNotInterestedMessage());
			}
		};

		auto OnPieceMessageReceived = [&](){
			uint32_t bytesSaved = 0;
			PieceParcel pieceParcel = msg.GetPiece();
			uint32_t pieceOffsetInFile = (pieceParcel.index * torrent.pieceLength) + pieceParcel.begin;
			
			fileHndl.Seek(pieceOffsetInFile);
			while (bytesSaved < bytesRequested &&
			       pieceOffsetInFile+bytesSaved < torrent.fileLength)
			{
				fileHndl.Put(pieceParcel.piece[bytesSaved]);
				bytesSaved++;
			}
			bytesTransferred += bytesSaved;
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
			messageHandlers[MessageType::HAVE] = UnExpected;
			messageHandlers[MessageType::BITFIELD] = OnBitfieldMessageReceived;
			messageHandlers[MessageType::REQUEST] = NotImplemented;
			messageHandlers[MessageType::PIECE] = OnPieceMessageReceived;
			messageHandlers[MessageType::CANCEL] = NotImplemented;
		}

		messagingSocket.SendMessage(MessageParcelFactory::GetUnChokedMessage());
		state.bSeederIsChokingLeecher = false;
		
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
}