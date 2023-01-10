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
}

namespace BT
{
#if 0
	namespace 
	{
		std::string receiveString(Peer const& peer, unsigned int const nBytesToRead) 
		{
			char buffer[Defaults::MaxBufferSize] = "";
			peer.Receive(static_cast<void*>(buffer), nBytesToRead);
			buffer[nBytesToRead] = '\0';

			return std::string(buffer);
		}

		long const receiveLong(Peer const& peer) 
		{
			unsigned int const nBytesToRead = sizeof(long);
			auto buffer = receiveString(peer, nBytesToRead);
			return std::stol(buffer);
		}

		unsigned long const receiveLengthOfMessage(Peer const& peer) 
		{
			unsigned int const nBytesToRead = sizeof(unsigned long);
			auto buffer = receiveString(peer, nBytesToRead);
			return std::stoul(buffer);
		}

		MessageType const receiveTypeOfMessage(Peer const& peer) 
		{
			unsigned int const nBytesToRead = 1;
			auto buffer = receiveString(peer, nBytesToRead);
			return MessageType(std::stoi(buffer));
		}

		template<MessageType msgType>
		MessageParcel const receiveSpecificMessage(Peer const& peer) 
		{
			if (receiveLengthOfMessage(peer) == 0)
			{
				return MessageParcelFactory::GetKeepAliveMessage();
			} 

			if (receiveTypeOfMessage(peer) != msgType)
			{
				throw UnExpectedMessage();
			}

			std::unordered_map<MessageType, std::function<MessageParcel const()>> handlers;
			handlers[MessageType::CHOKE] = MessageParcelFactory::GetChokedMessage;
			handlers[MessageType::UNCHOKE] = MessageParcelFactory::GetUnChokedMessage;
			handlers[MessageType::INTERESTED] = MessageParcelFactory::GetInterestedMessage;
			handlers[MessageType::NOTINTERESTED] = MessageParcelFactory::GetNotInterestedMessage;

			auto itr = handlers.find(msgType);
			if (itr != handlers.end())
			{ 
				throw UnExpectedMessage();
			}

			return itr->second();
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::HAVE>(Peer const& peer) 
		{
			if (receiveLengthOfMessage(peer) == 0)
			{
				return MessageParcelFactory::GetKeepAliveMessage();
			} 

			if(receiveTypeOfMessage(peer) != MessageType::HAVE)
			{ 
				throw UnExpectedMessage();
			}

			unsigned int const nBytesToRead = sizeof(long);
			auto buffer = receiveString(peer, nBytesToRead);
			return MessageParcelFactory::GetHaveMessage(std::stol(buffer));
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::BITFIELD>(Peer const& peer) 
		{
			auto msgLength = receiveLengthOfMessage(peer);
			if (msgLength == 0)
			{ 
				return MessageParcelFactory::GetKeepAliveMessage();
			}

			if (receiveTypeOfMessage(peer) != MessageType::HAVE) 
			{
				throw UnExpectedMessage();
			}

			auto buffer = receiveString(peer, msgLength);
			return  MessageParcelFactory::GetBitfieldMessage(buffer);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::REQUEST>(Peer const& peer) 
		{
			if (receiveLengthOfMessage(peer) == 0)
			{ 
				return MessageParcelFactory::GetKeepAliveMessage();
			}

			if (receiveTypeOfMessage(peer) != MessageType::HAVE) 
			{
				throw UnExpectedMessage();
			}

			RequestParcel request(receiveLong(peer), receiveLong(peer), receiveLong(peer));
			return  MessageParcelFactory::GetRequestMessage(request);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::PIECE>(Peer const& peer) 
		{
			if (receiveLengthOfMessage(peer) == 0)
			{ 
				return MessageParcelFactory::GetKeepAliveMessage();
			}

			if (receiveTypeOfMessage(peer) != MessageType::HAVE) 
			{
				throw UnExpectedMessage();
			}

			PieceParcel piece(receiveLong(peer), receiveLong(peer), nullptr);
			return  MessageParcelFactory::GetPieceMessage(piece);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::CANCEL>(Peer const& peer) 
		{
			if (receiveLengthOfMessage(peer) == 0) 
			{
				return MessageParcelFactory::GetKeepAliveMessage();
			}

			if (receiveTypeOfMessage(peer) != MessageType::HAVE)
			{ 
				throw UnExpectedMessage();
			}

			RequestParcel cancel(receiveLong(peer), receiveLong(peer), receiveLong(peer));
			return  MessageParcelFactory::GetCancelMessage(cancel);
		}

		void sendMessageLength(Peer const &peer, MessageParcel const &msg) 
		{
			char buffer[Defaults::MaxBufferSize] = "";

			auto msgLength = msg.GetLength();
			memcpy(buffer, &msgLength, sizeof(msgLength));
			peer.Send(buffer, sizeof(msgLength));
		}

		void sendMessageType(Peer const &peer, MessageParcel const &msg) 
		{
			// char buffer[Defaults::MaxBufferSize] = "";
			// ??
		}

		void sendLong(Peer const& peer, long const value) 
		{
			char buffer[Defaults::MaxBufferSize] = "";
			memcpy(buffer, &value, sizeof(value));
			peer.Send(buffer, sizeof(value));
		}

		void sendMessageAttributes(Peer const &peer, MessageParcel const &msg) 
		{
			sendMessageLength(peer, msg);
			sendMessageType(peer, msg);
		}

		template<MessageType msgType>
		void sendSpecificMessage(Peer const &peer, MessageParcel const &msg) {}

		template<>
		void sendSpecificMessage<MessageType::CHOKE>(Peer const &peer, MessageParcel const &msg) 
		{
			sendMessageAttributes(peer, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::UNCHOKE>(Peer const &peer, MessageParcel const &msg) 
		{
			sendMessageAttributes(peer, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::INTERESTED>(Peer const &peer, MessageParcel const &msg) 
		{
			sendMessageLength(peer, msg);
			if (msg.GetLength() == 0) 
			{
				return; /* Keepalive */
			}
			sendMessageType(peer, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::NOTINTERESTED>(Peer const &peer, MessageParcel const &msg) 
		{
			sendMessageAttributes(peer, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::HAVE>(Peer const &peer, MessageParcel const &msg) 
		{
			sendMessageAttributes(peer, msg);
			sendLong(peer, msg.GetHave());
		}

		template<>
		void sendSpecificMessage<MessageType::BITFIELD>(Peer const &peer, MessageParcel const &msg) 
		{
			sendMessageAttributes(peer, msg);

			auto bitfield = msg.GetBitfield();
			char buffer[Defaults::MaxBufferSize] = "";

			strcpy(buffer, bitfield.c_str());
			peer.Send(buffer, bitfield.length());
		}

		template<>
		void sendSpecificMessage<MessageType::REQUEST>(Peer const &peer, MessageParcel const &msg) 
		{
			sendMessageAttributes(peer, msg);

			auto request = msg.GetRequest();
			sendLong(peer, request.index);
			sendLong(peer, request.begin);
			sendLong(peer, request.length);
		}

		template<>
		void sendSpecificMessage<MessageType::PIECE>(Peer const &peer, MessageParcel const &msg) 
		{
			sendMessageAttributes(peer, msg);

			auto piece = msg.GetPiece();
			sendLong(peer, piece.index);
			sendLong(peer, piece.begin);
		}

		template<>
		void sendSpecificMessage<MessageType::CANCEL>(Peer const &peer, MessageParcel const &msg) 
		{
			sendMessageAttributes(peer, msg);

			auto cancel = msg.GetRequest();
			sendLong(peer, cancel.index);
			sendLong(peer, cancel.begin);
			sendLong(peer, cancel.length);
		}
	}
#endif

	Peer::Peer()
		: port(0)
	{}

	Peer::Peer(unsigned int const port)
		: ip("localhost"),
		  port(port),
		  id(CalculateId(ip, port)) 
	{}

	Peer::Peer(std::string const& ip, unsigned int const port)
		: ip(ip),
		  port(port),
		  id(CalculateId(ip, port)) 
	{}

	Peer::Peer(Peer const& otherPeer)
		: ip(otherPeer.ip), 
		  port(otherPeer.port),
		  id(otherPeer.id) 
	{}

	Peer::Peer(Peer&& otherPeer)
	{
		swap(*this, otherPeer);
	}

	Peer& Peer::operator=(Peer otherPeer) 
	{
		swap(*this, otherPeer);
		return *this;
	}

	void swap(Peer& first, Peer& second)
	{
		std::swap(first.ip, second.ip);
		std::swap(first.port, second.port);
		std::swap(first.id, second.id);
	}

	void Peer::reset() 
	{
		ip = "";
		port = 0;
		id = "";
	}

	std::ostream& operator<<(std::ostream& os, Peer const& peer) 
	{
		os << peer.id << "        " << peer.ip << ":" << peer.port;
		return os;
	}
	
#if 0
	MessageParcel const Peer::ReceiveMessage(MessageType const msgType) const {
		std::unordered_map<MessageType, std::function<MessageParcel const(Peer const&)>> handlers;

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
		if (itr == handlers.end())
		{
			throw UnExpectedMessage();
		}
		return itr->second(*this);		
	}

	void Peer::SendMessage(MessageParcel const& msg) const {
		std::unordered_map<MessageType, std::function<void(Peer const& peer, MessageParcel const&)>> handlers;

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
		if (itr == handlers.end())
		{
			throw UnExpectedMessage();
		}
		itr->second(*this, msg);
	}
#endif
}