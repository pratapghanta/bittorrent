#include <iostream>
#include <string>
#include <algorithm>
#include <exception>
#include <utility>
#include <cstring>
#include <ctime>
#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <functional>
#include <netinet/in.h>
#include <netinet/ip.h> //ip hdeader library (must come before ip_icmp.h)
#include <netinet/ip_icmp.h> //icmp header
#include <arpa/inet.h> //internet address library
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <openssl/sha.h>

#include "common/Defaults.hpp"
#include "common/Errors.hpp"
#include "common/Helpers.hpp"
#include "peer/Peer.hpp"
#include "peer/MessageParcelFactory.hpp"

namespace 
{
	struct UnExpectedMessage : BT::CException 
	{
		UnExpectedMessage()
            : BT::CException("Malformed message is being communicated.") 
        {}
	};

	struct BadSocketDescriptor : BT::CException 
	{
		BadSocketDescriptor()
            : BT::CException("Bad socket descriptor.") 
        {}
	};

	struct UnableToCreateSocket : BT::CException 
	{
		UnableToCreateSocket()
            : BT::CException("Unable to create a socket.") 
        {}
	};

	struct SocketNonBlockable : BT::CException 
	{
		SocketNonBlockable()
            : BT::CException("Unable to make socket non-blockable.") 
        {}
	};

	void makeSocketNonBlockable(int sockfd) 
	{
		int flags = fcntl(sockfd, F_GETFL, 0);
		if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK))
			throw SocketNonBlockable();
	}

	std::string receiveString(BT::Peer const& peer, unsigned int const nBytesToRead) 
	{
		char buffer[BT::Defaults::MaxBufferSize] = "";
		peer.Receive(static_cast<void*>(buffer), nBytesToRead);
		buffer[nBytesToRead] = '\0';

		return std::string(buffer);
	}

	long const receiveLong(BT::Peer const& peer) 
	{
		unsigned int const nBytesToRead = sizeof(long);
		auto buffer = receiveString(peer, nBytesToRead);
		return std::stol(buffer);
	}

	unsigned long const receiveLengthOfMessage(BT::Peer const& peer) 
	{
		unsigned int const nBytesToRead = sizeof(unsigned long);
		auto buffer = receiveString(peer, nBytesToRead);
		return std::stoul(buffer);
	}

	BT::MessageType const receiveTypeOfMessage(BT::Peer const& peer) 
	{
		unsigned int const nBytesToRead = 1;
		auto buffer = receiveString(peer, nBytesToRead);
		return BT::MessageType(std::stoi(buffer));
	}

	template<BT::MessageType msgType>
	BT::MessageParcel const receiveSpecificMessage(BT::Peer const& peer) 
	{
		if (receiveLengthOfMessage(peer) == 0)
		{
			return BT::MessageParcelFactory::GetKeepAliveMessage();
		} 

		if (receiveTypeOfMessage(peer) != msgType)
		{
			throw UnExpectedMessage();
		}

		std::unordered_map<BT::MessageType, std::function<BT::MessageParcel const()>> handlers;
		handlers[BT::MessageType::CHOKE] = BT::MessageParcelFactory::GetChokedMessage;
		handlers[BT::MessageType::UNCHOKE] = BT::MessageParcelFactory::GetUnChokedMessage;
		handlers[BT::MessageType::INTERESTED] = BT::MessageParcelFactory::GetInterestedMessage;
		handlers[BT::MessageType::NOTINTERESTED] = BT::MessageParcelFactory::GetNotInterestedMessage;

		auto itr = handlers.find(msgType);
		if (itr != handlers.end())
		{ 
			throw UnExpectedMessage();
		}

		return itr->second();
	}

	template<>
	BT::MessageParcel const receiveSpecificMessage<BT::MessageType::HAVE>(BT::Peer const& peer) 
	{
		if (receiveLengthOfMessage(peer) == 0)
		{
			return BT::MessageParcelFactory::GetKeepAliveMessage();
		} 

		if(receiveTypeOfMessage(peer) != BT::MessageType::HAVE)
		{ 
			throw UnExpectedMessage();
		}

		unsigned int const nBytesToRead = sizeof(long);
		auto buffer = receiveString(peer, nBytesToRead);
		return BT::MessageParcelFactory::GetHaveMessage(std::stol(buffer));
	}

	template<>
	BT::MessageParcel const receiveSpecificMessage<BT::MessageType::BITFIELD>(BT::Peer const& peer) 
	{
		auto msgLength = receiveLengthOfMessage(peer);
		if (msgLength == 0)
		{ 
			return BT::MessageParcelFactory::GetKeepAliveMessage();
		}

		if (receiveTypeOfMessage(peer) != BT::MessageType::HAVE) 
		{
			throw UnExpectedMessage();
		}

		auto buffer = receiveString(peer, msgLength);
		return  BT::MessageParcelFactory::GetBitfieldMessage(buffer);
	}

	template<>
	BT::MessageParcel const receiveSpecificMessage<BT::MessageType::REQUEST>(BT::Peer const& peer) 
	{
		if (receiveLengthOfMessage(peer) == 0)
		{ 
			return BT::MessageParcelFactory::GetKeepAliveMessage();
		}

		if (receiveTypeOfMessage(peer) != BT::MessageType::HAVE) 
		{
			throw UnExpectedMessage();
		}

		BT::RequestParcel request(receiveLong(peer), receiveLong(peer), receiveLong(peer));
		return  BT::MessageParcelFactory::GetRequestMessage(request);
	}

	template<>
	BT::MessageParcel const receiveSpecificMessage<BT::MessageType::PIECE>(BT::Peer const& peer) 
	{
		if (receiveLengthOfMessage(peer) == 0)
		{ 
			return BT::MessageParcelFactory::GetKeepAliveMessage();
		}

		if (receiveTypeOfMessage(peer) != BT::MessageType::HAVE) 
		{
			throw UnExpectedMessage();
		}

		BT::PieceParcel piece(receiveLong(peer), receiveLong(peer), nullptr);
		return  BT::MessageParcelFactory::GetPieceMessage(piece);
	}

	template<>
	BT::MessageParcel const receiveSpecificMessage<BT::MessageType::CANCEL>(BT::Peer const& peer) 
	{
		if (receiveLengthOfMessage(peer) == 0) 
		{
			return BT::MessageParcelFactory::GetKeepAliveMessage();
		}

		if (receiveTypeOfMessage(peer) != BT::MessageType::HAVE)
		{ 
			throw UnExpectedMessage();
		}

		BT::RequestParcel cancel(receiveLong(peer), receiveLong(peer), receiveLong(peer));
		return  BT::MessageParcelFactory::GetCancelMessage(cancel);
	}

	void sendMessageLength(BT::Peer const &peer, BT::MessageParcel const &msg) 
	{
		char buffer[BT::Defaults::MaxBufferSize] = "";

		auto msgLength = msg.GetLength();
		memcpy(buffer, &msgLength, sizeof(msgLength));
		peer.Send(buffer, sizeof(msgLength));
	}

	void sendMessageType(BT::Peer const &peer, BT::MessageParcel const &msg) 
	{
		// char buffer[BT::Defaults::MaxBufferSize] = "";
		// ??
	}

	void sendLong(BT::Peer const& peer, long const value) 
	{
		char buffer[BT::Defaults::MaxBufferSize] = "";
		memcpy(buffer, &value, sizeof(value));
		peer.Send(buffer, sizeof(value));
	}

	void sendMessageAttributes(BT::Peer const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageLength(peer, msg);
		sendMessageType(peer, msg);
	}

	template<BT::MessageType msgType>
	void sendSpecificMessage(BT::Peer const &peer, BT::MessageParcel const &msg) {}

	template<>
	void sendSpecificMessage<BT::MessageType::CHOKE>(BT::Peer const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);
	}

	template<>
	void sendSpecificMessage<BT::MessageType::UNCHOKE>(BT::Peer const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);
	}

	template<>
	void sendSpecificMessage<BT::MessageType::INTERESTED>(BT::Peer const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageLength(peer, msg);
		if (msg.GetLength() == 0) 
		{
			return; /* Keepalive */
		}
		sendMessageType(peer, msg);
	}

	template<>
	void sendSpecificMessage<BT::MessageType::NOTINTERESTED>(BT::Peer const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);
	}

	template<>
	void sendSpecificMessage<BT::MessageType::HAVE>(BT::Peer const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);
		sendLong(peer, msg.GetHave());
	}

	template<>
	void sendSpecificMessage<BT::MessageType::BITFIELD>(BT::Peer const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);

		auto bitfield = msg.GetBitfield();
		char buffer[BT::Defaults::MaxBufferSize] = "";

		strcpy(buffer, bitfield.c_str());
		peer.Send(buffer, bitfield.length());
	}

	template<>
	void sendSpecificMessage<BT::MessageType::REQUEST>(BT::Peer const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);

		auto request = msg.GetRequest();
		sendLong(peer, request.index);
		sendLong(peer, request.begin);
		sendLong(peer, request.length);
	}

	template<>
	void sendSpecificMessage<BT::MessageType::PIECE>(BT::Peer const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);

		auto piece = msg.GetPiece();
		sendLong(peer, piece.index);
		sendLong(peer, piece.begin);
	}

	template<>
	void sendSpecificMessage<BT::MessageType::CANCEL>(BT::Peer const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);

		auto cancel = msg.GetRequest();
		sendLong(peer, cancel.index);
		sendLong(peer, cancel.begin);
		sendLong(peer, cancel.length);
	}
}

BT::Peer::Peer()
	: sockfd(BT::Defaults::BadFD),
	  port(0)
{}

BT::Peer::Peer(int const& fd, std::string const& ip, unsigned int const& port) 
     : sockfd(BT::Defaults::BadFD), 
	   ip(ip),
	   port(port), 
	   id(CalculateId(ip, port)) 
{
	if (fd == BT::Defaults::BadFD)
		return;

	sockfd = dup(fd);
	if (fd == BT::Defaults::BadFD) 
		throw BadSocketDescriptor();
}

BT::Peer::Peer(std::string const& ip, unsigned int const& port)
	: Peer(BT::Defaults::BadFD, ip, port) 
{}

BT::Peer::Peer(Peer const& otherPeer)
	: ip(otherPeer.ip), 
	  port(otherPeer.port),
	  id(otherPeer.id) 
{
	if (sockfd != BT::Defaults::BadFD)
		close(sockfd);

	sockfd = dup(otherPeer.sockfd);
	if (sockfd == BT::Defaults::BadFD)
		throw BadSocketDescriptor();
}

BT::Peer::Peer(BT::Peer&& otherPeer) 
	: sockfd(BT::Defaults::BadFD)
{
	swap(*this, otherPeer);
}

BT::Peer& BT::Peer::operator=(BT::Peer otherPeer) 
{
	std::swap(*this, otherPeer);
	return *this;
}

void BT::swap(BT::Peer& first, BT::Peer& second)
{
	std::swap(first.sockfd, second.sockfd);
	std::swap(first.ip, second.ip);
	std::swap(first.port, second.port);
	std::swap(first.id, second.id);
}

BT::Peer::~Peer() 
{
	if (sockfd != BT::Defaults::BadFD)
		close(sockfd);
	sockfd = BT::Defaults::BadFD;
}

void BT::Peer::Reset() 
{
	if (sockfd != BT::Defaults::BadFD)
		close(sockfd);

	sockfd = BT::Defaults::BadFD;
	ip = "";
	port = 0;
	id = "";
}

void BT::Peer::EstablishConnectionTo(Peer const& otherPeer) {
	sockfd = BT::Defaults::BadFD;

	int tmpSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tmpSockfd < 0)
		return;

	sockaddr_in serv_addr;
	memset((void *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(otherPeer.ip.c_str());
	serv_addr.sin_port = htons(otherPeer.port);

	if (connect(tmpSockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) != 0)
		return;

	sockaddr_in cli_addr;
	socklen_t len = BT::Defaults::IPSize - 1;

	if (getsockname(tmpSockfd, (sockaddr*)&cli_addr, &len) == -1)
		return;

	char buffer[BT::Defaults::IPSize] = "";
	inet_ntop(cli_addr.sin_family, (void *)&cli_addr.sin_addr, buffer, BT::Defaults::IPSize - 1);

	sockfd = tmpSockfd;
	ip = std::string(buffer);
	port = ntohs(cli_addr.sin_port);
	id = CalculateId(ip, port);
}

void BT::Peer::Receive(void *buf, unsigned int const count) const {
	memset(buf, 0, count);
	makeSocketNonBlockable(sockfd);

	unsigned int totalBytesRead = 0;
	time_t beg = time(0);

	while (totalBytesRead < count) {
		auto bufPtr = &(static_cast<char*>(buf)[totalBytesRead]);
		int const bytesRead = read(sockfd, bufPtr, 1);
		if (bytesRead > 0) {
			totalBytesRead += bytesRead;
			time(&beg);
			continue;
		}

		time_t end = time(0);
		if (difftime(end, beg) > 2 * 60)
			throw UnExpectedMessage();
	}
}

void BT::Peer::Send(void const * const buf, unsigned int const count) const {
	write(sockfd, buf, count);
}

BT::MessageParcel const BT::Peer::ReceiveMessage(BT::MessageType const msgType) const {
	std::unordered_map<BT::MessageType, std::function<MessageParcel const(Peer const&)>> handlers;

	handlers[MessageType::CHOKE] = receiveSpecificMessage<MessageType::CHOKE>;
	handlers[MessageType::UNCHOKE] = receiveSpecificMessage<MessageType::UNCHOKE>;
	handlers[MessageType::INTERESTED] = receiveSpecificMessage<MessageType::INTERESTED>;
	handlers[MessageType::NOTINTERESTED] = receiveSpecificMessage<MessageType::NOTINTERESTED>;
	handlers[MessageType::HAVE] = receiveSpecificMessage<MessageType::HAVE>;
	handlers[MessageType::BITFIELD] = receiveSpecificMessage<MessageType::BITFIELD>;
	handlers[MessageType::REQUEST] = receiveSpecificMessage<MessageType::REQUEST>;
	handlers[MessageType::PIECE] = receiveSpecificMessage<MessageType::PIECE>;
	handlers[MessageType::CANCEL] = receiveSpecificMessage<MessageType::CANCEL>;

	auto itr = handlers.find(msgType);
	if (itr == handlers.end()) throw UnExpectedMessage();
	return itr->second(*this);		
}

void BT::Peer::SendMessage(BT::MessageParcel const& msg) const {
	std::unordered_map<BT::MessageType, std::function<void(Peer const& peer, BT::MessageParcel const&)>> handlers;

	handlers[MessageType::CHOKE] = sendSpecificMessage<MessageType::CHOKE>;
	handlers[MessageType::UNCHOKE] = sendSpecificMessage<MessageType::UNCHOKE>;
	handlers[MessageType::INTERESTED] = sendSpecificMessage<MessageType::INTERESTED>;
	handlers[MessageType::NOTINTERESTED] = sendSpecificMessage<MessageType::NOTINTERESTED>;
	handlers[MessageType::HAVE] = sendSpecificMessage<MessageType::HAVE>;
	handlers[MessageType::BITFIELD] = sendSpecificMessage<MessageType::BITFIELD>;
	handlers[MessageType::REQUEST] = sendSpecificMessage<MessageType::REQUEST>;
	handlers[MessageType::PIECE] = sendSpecificMessage<MessageType::PIECE>;
	handlers[MessageType::CANCEL] = sendSpecificMessage<MessageType::CANCEL>;

	auto itr = handlers.find(msg.GetType());
	if (itr == handlers.end()) throw UnExpectedMessage();
	itr->second(*this, msg);
}

std::ostream& BT::operator<<(std::ostream& os, BT::Peer const& peer) {
	os << peer.id << "        " << peer.ip << ":" << peer.port;
	return os;
}