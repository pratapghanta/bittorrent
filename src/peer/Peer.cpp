#if 0
#include <iostream>
#include <string>
#include <algorithm>
#include <exception>
#include <utility>
#include <cstring>
#include <ctime>
#include <map>
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

	std::string receiveString(BT::Peer_t const& peer, unsigned int const nBytesToRead) 
	{
		char buffer[BT::Defaults::MaxBufferSize] = "";
		peer.Receive(static_cast<void*>(buffer), nBytesToRead);
		buffer[nBytesToRead] = '\0';

		return std::string(buffer);
	}

	long const receiveLong(BT::Peer_t const& peer) 
	{
		unsigned int const nBytesToRead = sizeof(long);
		auto buffer = receiveString(peer, nBytesToRead);
		return std::stol(buffer);
	}

	unsigned long const receiveLengthOfMessage(BT::Peer_t const& peer) 
	{
		unsigned int const nBytesToRead = sizeof(unsigned long);
		auto buffer = receiveString(peer, nBytesToRead);
		return std::stoul(buffer);
	}

	BT::MessageParcel::MessageType const receiveTypeOfMessage(BT::Peer_t const& peer) 
	{
		unsigned int const nBytesToRead = 1;
		auto buffer = receiveString(peer, nBytesToRead);
		return BT::MessageParcel::MessageType(std::stoi(buffer));
	}

	template<BT::MessageParcel::MessageType msgType>
	BT::MessageParcel const receiveSpecificMessage(BT::Peer_t const& peer) 
	{
		if (receiveLengthOfMessage(peer) == 0) 
			return BT::MessageParcel::getKeepAliveMessage();

		if (receiveTypeOfMessage(peer) != msgType)
			throw UnExpectedMessage();
		
		std::map<BT::MessageParcel::MessageType, std::function<BT::MessageParcel const()>> handlers;
		handlers[BT::MessageParcel::MessageType::CHOKE] = BT::MessageParcel::getChokedMessage;
		handlers[BT::MessageParcel::MessageType::UNCHOKE] = BT::MessageParcel::getUnChokedMessage;
		handlers[BT::MessageParcel::MessageType::INTERESTED] = BT::MessageParcel::getInterestedMessage;
		handlers[BT::MessageParcel::MessageType::NOTINTERESTED] = BT::MessageParcel::getNotInterestedMessage;

		auto itr = handlers.find(msgType);
		if (itr != handlers.end()) 
			throw UnExpectedMessage();

		return itr->second();
	}

	template<>
	BT::MessageParcel const receiveSpecificMessage<BT::MessageParcel::MessageType::HAVE>(BT::Peer_t const& peer) 
	{
		if (receiveLengthOfMessage(peer) == 0) 
			return BT::MessageParcel::getKeepAliveMessage();

		if(receiveTypeOfMessage(peer) != BT::MessageParcel::MessageType::HAVE) 
			throw UnExpectedMessage();

		unsigned int const nBytesToRead = sizeof(long);
		auto buffer = receiveString(peer, nBytesToRead);
		return BT::MessageParcel::getHaveMessage(std::stol(buffer));
	}

	template<>
	BT::MessageParcel const receiveSpecificMessage<BT::MessageParcel::MessageType::BITFIELD>(BT::Peer_t const& peer) 
	{
		auto msgLength = receiveLengthOfMessage(peer);
		if (msgLength == 0) 
			return BT::MessageParcel::getKeepAliveMessage();

		if (receiveTypeOfMessage(peer) != BT::MessageParcel::MessageType::HAVE) 
			throw UnExpectedMessage();

		auto buffer = receiveString(peer, msgLength);
		return  BT::MessageParcel::getBitfieldMessage(buffer);
	}

	template<>
	BT::MessageParcel const receiveSpecificMessage<BT::MessageParcel::MessageType::REQUEST>(BT::Peer_t const& peer) 
	{
		if (receiveLengthOfMessage(peer) == 0) 
			return BT::MessageParcel::getKeepAliveMessage();

		if (receiveTypeOfMessage(peer) != BT::MessageParcel::MessageType::HAVE) 
			throw UnExpectedMessage();

		BT::RequestParcel request(receiveLong(peer), receiveLong(peer), receiveLong(peer));
		return  BT::MessageParcel::getRequestMessage(request);
	}

	template<>
	BT::MessageParcel const receiveSpecificMessage<BT::MessageParcel::MessageType::PIECE>(BT::Peer_t const& peer) 
	{
		if (receiveLengthOfMessage(peer) == 0) 
			return BT::MessageParcel::getKeepAliveMessage();

		if (receiveTypeOfMessage(peer) != BT::MessageParcel::MessageType::HAVE) 
			throw UnExpectedMessage();

		BT::PieceParcel piece(receiveLong(peer), receiveLong(peer), nullptr);
		return  BT::MessageParcel::getPieceMessage(piece);
	}

	template<>
	BT::MessageParcel const receiveSpecificMessage<BT::MessageParcel::MessageType::CANCEL>(BT::Peer_t const& peer) 
	{
		if (receiveLengthOfMessage(peer) == 0) 
			return BT::MessageParcel::getKeepAliveMessage();

		if (receiveTypeOfMessage(peer) != BT::MessageParcel::MessageType::HAVE) 
			throw UnExpectedMessage();

		BT::RequestParcel cancel(receiveLong(peer), receiveLong(peer), receiveLong(peer));
		return  BT::MessageParcel::getCancelMessage(cancel);
	}

	void sendMessageLength(BT::Peer_t const &peer, BT::MessageParcel const &msg) 
	{
		char buffer[BT::Defaults::MaxBufferSize] = "";

		auto msgLength = msg.getLength();
		memcpy(buffer, &msgLength, sizeof(msgLength));
		peer.Send(buffer, sizeof(msgLength));
	}

	void sendMessageType(BT::Peer_t const &peer, BT::MessageParcel const &msg) 
	{
		// char buffer[BT::Defaults::MaxBufferSize] = "";
		// ??
	}

	void sendLong(BT::Peer_t const& peer, long const value) 
	{
		char buffer[BT::Defaults::MaxBufferSize] = "";
		memcpy(buffer, &value, sizeof(value));
		peer.Send(buffer, sizeof(value));
	}

	void sendMessageAttributes(BT::Peer_t const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageLength(peer, msg);
		sendMessageType(peer, msg);
	}

	template<BT::MessageParcel::MessageType msgType>
	void sendSpecificMessage(BT::Peer_t const &peer, BT::MessageParcel const &msg) {}

	template<>
	void sendSpecificMessage<BT::MessageParcel::MessageType::CHOKE>(BT::Peer_t const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);
	}

	template<>
	void sendSpecificMessage<BT::MessageParcel::MessageType::UNCHOKE>(BT::Peer_t const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);
	}

	template<>
	void sendSpecificMessage<BT::MessageParcel::MessageType::INTERESTED>(BT::Peer_t const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageLength(peer, msg);
		if (msg.getLength() == 0) return; /* Keepalive */
		sendMessageType(peer, msg);
	}

	template<>
	void sendSpecificMessage<BT::MessageParcel::MessageType::NOTINTERESTED>(BT::Peer_t const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);
	}

	template<>
	void sendSpecificMessage<BT::MessageParcel::MessageType::HAVE>(BT::Peer_t const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);
		sendLong(peer, msg.getHave());
	}

	template<>
	void sendSpecificMessage<BT::MessageParcel::MessageType::BITFIELD>(BT::Peer_t const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);

		auto bitfield = msg.getBitfield();
		char buffer[BT::Defaults::MaxBufferSize] = "";

		strcpy(buffer, bitfield.c_str());
		peer.Send(buffer, bitfield.length());
	}

	template<>
	void sendSpecificMessage<BT::MessageParcel::MessageType::REQUEST>(BT::Peer_t const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);

		auto request = msg.getRequest();
		sendLong(peer, request.index);
		sendLong(peer, request.begin);
		sendLong(peer, request.length);
	}

	template<>
	void sendSpecificMessage<BT::MessageParcel::MessageType::PIECE>(BT::Peer_t const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);

		auto piece = msg.getPiece();
		sendLong(peer, piece.index);
		sendLong(peer, piece.begin);
	}

	template<>
	void sendSpecificMessage<BT::MessageParcel::MessageType::CANCEL>(BT::Peer_t const &peer, BT::MessageParcel const &msg) 
	{
		sendMessageAttributes(peer, msg);

		auto cancel = msg.getRequest();
		sendLong(peer, cancel.index);
		sendLong(peer, cancel.begin);
		sendLong(peer, cancel.length);
	}
}

BT::Peer_t::Peer_t()
	: sockfd(BT::Defaults::BadFD),
	  port(0)
{}

BT::Peer_t::Peer_t(int const& fd, std::string const& ip, unsigned int const& port) 
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

BT::Peer_t::Peer_t(std::string const& ip, unsigned int const& port)
	: Peer_t(BT::Defaults::BadFD, ip, port) 
{}

BT::Peer_t::Peer_t(Peer_t const& otherPeer)
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

BT::Peer_t::Peer_t(BT::Peer_t&& otherPeer) 
	: sockfd(BT::Defaults::BadFD)
{
	swap(*this, otherPeer);
}

BT::Peer_t& BT::Peer_t::operator=(BT::Peer_t otherPeer) 
{
	std::swap(*this, otherPeer);
	return *this;
}

void BT::swap(BT::Peer_t& first, BT::Peer_t& second)
{
	std::swap(first.sockfd, second.sockfd);
	std::swap(first.ip, second.ip);
	std::swap(first.port, second.port);
	std::swap(first.id, second.id);
}

BT::Peer_t::~Peer_t() 
{
	if (sockfd != BT::Defaults::BadFD)
		close(sockfd);
	sockfd = BT::Defaults::BadFD;
}

void BT::Peer_t::Reset() 
{
	if (sockfd != BT::Defaults::BadFD)
		close(sockfd);

	sockfd = BT::Defaults::BadFD;
	ip = "";
	port = 0;
	id = "";
}

void BT::Peer_t::EstablishConnectionTo(Peer_t const& otherPeer) {
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

void BT::Peer_t::Receive(void *buf, unsigned int const count) const {
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

void BT::Peer_t::Send(void const * const buf, unsigned int const count) const {
	write(sockfd, buf, count);
}

BT::MessageParcel const BT::Peer_t::ReceiveMessage(BT::MessageParcel::MessageType const msgType) const {
	std::map<MessageParcel::MessageType, std::function<MessageParcel const(Peer_t const&)>> handlers;

	handlers[MessageParcel::MessageType::CHOKE] = receiveSpecificMessage<MessageParcel::MessageType::CHOKE>;
	handlers[MessageParcel::MessageType::UNCHOKE] = receiveSpecificMessage<MessageParcel::MessageType::UNCHOKE>;
	handlers[MessageParcel::MessageType::INTERESTED] = receiveSpecificMessage<MessageParcel::MessageType::INTERESTED>;
	handlers[MessageParcel::MessageType::NOTINTERESTED] = receiveSpecificMessage<MessageParcel::MessageType::NOTINTERESTED>;
	handlers[MessageParcel::MessageType::HAVE] = receiveSpecificMessage<MessageParcel::MessageType::HAVE>;
	handlers[MessageParcel::MessageType::BITFIELD] = receiveSpecificMessage<MessageParcel::MessageType::BITFIELD>;
	handlers[MessageParcel::MessageType::REQUEST] = receiveSpecificMessage<MessageParcel::MessageType::REQUEST>;
	handlers[MessageParcel::MessageType::PIECE] = receiveSpecificMessage<MessageParcel::MessageType::PIECE>;
	handlers[MessageParcel::MessageType::CANCEL] = receiveSpecificMessage<MessageParcel::MessageType::CANCEL>;

	auto itr = handlers.find(msgType);
	if (itr == handlers.end()) throw UnExpectedMessage();
	return itr->second(*this);		
}

void BT::Peer_t::SendMessage(BT::MessageParcel const& msg) const {
	std::map<MessageParcel::MessageType, std::function<void(Peer_t const& peer, BT::MessageParcel const&)>> handlers;

	handlers[MessageParcel::MessageType::CHOKE] = sendSpecificMessage<MessageParcel::MessageType::CHOKE>;
	handlers[MessageParcel::MessageType::UNCHOKE] = sendSpecificMessage<MessageParcel::MessageType::UNCHOKE>;
	handlers[MessageParcel::MessageType::INTERESTED] = sendSpecificMessage<MessageParcel::MessageType::INTERESTED>;
	handlers[MessageParcel::MessageType::NOTINTERESTED] = sendSpecificMessage<MessageParcel::MessageType::NOTINTERESTED>;
	handlers[MessageParcel::MessageType::HAVE] = sendSpecificMessage<MessageParcel::MessageType::HAVE>;
	handlers[MessageParcel::MessageType::BITFIELD] = sendSpecificMessage<MessageParcel::MessageType::BITFIELD>;
	handlers[MessageParcel::MessageType::REQUEST] = sendSpecificMessage<MessageParcel::MessageType::REQUEST>;
	handlers[MessageParcel::MessageType::PIECE] = sendSpecificMessage<MessageParcel::MessageType::PIECE>;
	handlers[MessageParcel::MessageType::CANCEL] = sendSpecificMessage<MessageParcel::MessageType::CANCEL>;

	auto itr = handlers.find(msg.getType());
	if (itr == handlers.end()) throw UnExpectedMessage();
	itr->second(*this, msg);
}

std::ostream& BT::operator<<(std::ostream& os, BT::Peer_t const& peer) {
	os << peer.id << "        " << peer.ip << ":" << peer.port;
}

#endif