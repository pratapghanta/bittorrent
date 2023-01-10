#include <cstring>
#include <functional>
#include <unordered_map>

#include "common/Defaults.hpp"
#include "common/Errors.hpp"
#include "common/Helpers.hpp"
#include "peer/MessageParcel.hpp"
#include "peer/MessageParcelFactory.hpp"
#include "peer/MessagingSocket.hpp"
#include "socket/ConnectedSocketParcel.hpp"

namespace 
{
	struct UnExpectedMessage : BT::CException 
	{
		UnExpectedMessage()
            : BT::CException("Malformed message is being communicated.") 
        {}
	};
}

namespace BT
{
	namespace 
	{
		std::string receiveString(ConnectedSocket const& socket, unsigned int const nBytesToRead) 
		{
			char buffer[Defaults::MaxBufferSize] = "";
			socket.Receive(buffer, nBytesToRead);
			buffer[nBytesToRead] = '\0';

			return std::string(buffer);
		}

		long const receiveLong(ConnectedSocket const& socket) 
		{
			unsigned int const nBytesToRead = sizeof(long);
			auto buffer = receiveString(socket, nBytesToRead);
			return std::stol(buffer);
		}

		unsigned long const receiveLengthOfMessage(ConnectedSocket const& socket) 
		{
			unsigned int const nBytesToRead = sizeof(unsigned long);
			auto buffer = receiveString(socket, nBytesToRead);
			return std::stoul(buffer);
		}

		MessageType const receiveTypeOfMessage(ConnectedSocket const& socket) 
		{
			unsigned int const nBytesToRead = 1;
			auto buffer = receiveString(socket, nBytesToRead);
			return MessageType(std::stoi(buffer));
		}

		template<MessageType msgType>
		MessageParcel const receiveSpecificMessage(ConnectedSocket const& socket) 
		{
			if (receiveLengthOfMessage(socket) == 0)
			{
				return MessageParcelFactory::GetKeepAliveMessage();
			} 

			if (receiveTypeOfMessage(socket) != msgType)
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
		MessageParcel const receiveSpecificMessage<MessageType::HAVE>(ConnectedSocket const& socket) 
		{
			if (receiveLengthOfMessage(socket) == 0)
			{
				return MessageParcelFactory::GetKeepAliveMessage();
			} 

			if(receiveTypeOfMessage(socket) != MessageType::HAVE)
			{ 
				throw UnExpectedMessage();
			}

			unsigned int const nBytesToRead = sizeof(long);
			auto buffer = receiveString(socket, nBytesToRead);
			return MessageParcelFactory::GetHaveMessage(std::stol(buffer));
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::BITFIELD>(ConnectedSocket const& socket) 
		{
			auto msgLength = receiveLengthOfMessage(socket);
			if (msgLength == 0)
			{ 
				return MessageParcelFactory::GetKeepAliveMessage();
			}

			if (receiveTypeOfMessage(socket) != MessageType::HAVE) 
			{
				throw UnExpectedMessage();
			}

			auto buffer = receiveString(socket, msgLength);
			return  MessageParcelFactory::GetBitfieldMessage(buffer);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::REQUEST>(ConnectedSocket const& socket) 
		{
			if (receiveLengthOfMessage(socket) == 0)
			{ 
				return MessageParcelFactory::GetKeepAliveMessage();
			}

			if (receiveTypeOfMessage(socket) != MessageType::HAVE) 
			{
				throw UnExpectedMessage();
			}

			RequestParcel request(receiveLong(socket), receiveLong(socket), receiveLong(socket));
			return  MessageParcelFactory::GetRequestMessage(request);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::PIECE>(ConnectedSocket const& socket) 
		{
			if (receiveLengthOfMessage(socket) == 0)
			{ 
				return MessageParcelFactory::GetKeepAliveMessage();
			}

			if (receiveTypeOfMessage(socket) != MessageType::HAVE) 
			{
				throw UnExpectedMessage();
			}

			PieceParcel piece(receiveLong(socket), receiveLong(socket), nullptr);
			return  MessageParcelFactory::GetPieceMessage(piece);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::CANCEL>(ConnectedSocket const& socket) 
		{
			if (receiveLengthOfMessage(socket) == 0) 
			{
				return MessageParcelFactory::GetKeepAliveMessage();
			}

			if (receiveTypeOfMessage(socket) != MessageType::HAVE)
			{ 
				throw UnExpectedMessage();
			}

			RequestParcel cancel(receiveLong(socket), receiveLong(socket), receiveLong(socket));
			return  MessageParcelFactory::GetCancelMessage(cancel);
		}

		void sendMessageLength(ConnectedSocket const& socket, MessageParcel const &msg) 
		{
			char buffer[Defaults::MaxBufferSize] = "";

			auto msgLength = msg.GetLength();
			memcpy(buffer, &msgLength, sizeof(msgLength));
			socket.Send(buffer, sizeof(msgLength));
		}

		void sendMessageType(ConnectedSocket const& socket, MessageParcel const &msg) 
		{
			// char buffer[Defaults::MaxBufferSize] = "";
			// ??
		}

		void sendLong(ConnectedSocket const& socket, long const value) 
		{
			char buffer[Defaults::MaxBufferSize] = "";
			memcpy(buffer, &value, sizeof(value));
			socket.Send(buffer, sizeof(value));
		}

		void sendMessageAttributes(ConnectedSocket const& socket, MessageParcel const &msg) 
		{
			sendMessageLength(socket, msg);
			sendMessageType(socket, msg);
		}

		template<MessageType msgType>
		void sendSpecificMessage(ConnectedSocket const& socket, MessageParcel const &msg) {}

		template<>
		void sendSpecificMessage<MessageType::CHOKE>(ConnectedSocket const& socket, MessageParcel const &msg) 
		{
			sendMessageAttributes(socket, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::UNCHOKE>(ConnectedSocket const& socket, MessageParcel const &msg) 
		{
			sendMessageAttributes(socket, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::INTERESTED>(ConnectedSocket const& socket, MessageParcel const &msg) 
		{
			sendMessageLength(socket, msg);
			if (msg.GetLength() == 0) 
			{
				return; /* Keepalive */
			}
			sendMessageType(socket, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::NOTINTERESTED>(ConnectedSocket const& socket, MessageParcel const &msg) 
		{
			sendMessageAttributes(socket, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::HAVE>(ConnectedSocket const& socket, MessageParcel const &msg) 
		{
			sendMessageAttributes(socket, msg);
			sendLong(socket, msg.GetHave());
		}

		template<>
		void sendSpecificMessage<MessageType::BITFIELD>(ConnectedSocket const& socket, MessageParcel const &msg) 
		{
			sendMessageAttributes(socket, msg);

			auto bitfield = msg.GetBitfield();
			char buffer[Defaults::MaxBufferSize] = "";

			strcpy(buffer, bitfield.c_str());
			socket.Send(buffer, bitfield.length());
		}

		template<>
		void sendSpecificMessage<MessageType::REQUEST>(ConnectedSocket const& socket, MessageParcel const &msg) 
		{
			sendMessageAttributes(socket, msg);

			auto request = msg.GetRequest();
			sendLong(socket, request.index);
			sendLong(socket, request.begin);
			sendLong(socket, request.length);
		}

		template<>
		void sendSpecificMessage<MessageType::PIECE>(ConnectedSocket const& socket, MessageParcel const &msg) 
		{
			sendMessageAttributes(socket, msg);

			auto piece = msg.GetPiece();
			sendLong(socket, piece.index);
			sendLong(socket, piece.begin);
		}

		template<>
		void sendSpecificMessage<MessageType::CANCEL>(ConnectedSocket const& socket, MessageParcel const &msg) 
		{
			sendMessageAttributes(socket, msg);

			auto cancel = msg.GetRequest();
			sendLong(socket, cancel.index);
			sendLong(socket, cancel.begin);
			sendLong(socket, cancel.length);
		}
	}
}

namespace BT 
{
    MessagingSocket::MessagingSocket(ConnectedSocketParcel const& parcel)
        : socket(parcel)
    {
        fromId = CalculateId(parcel.fromIp, parcel.fromPort);
        toId = CalculateId(parcel.toIp, parcel.toPort);
    }

    int MessagingSocket::Send(char const * const buffer, unsigned int count) const
    {
        return socket.Send(buffer, count);
    }

    int MessagingSocket::Receive(char* buffer, unsigned int count) const
    {
        return socket.Receive(buffer, count);
    }

    void MessagingSocket::SendMessage(MessageParcel const& msg) const
    {
        static std::unordered_map<MessageType, std::function<void(ConnectedSocket const&, MessageParcel const&)>> handlers;
        if (handlers.empty())
        {
            handlers[MessageType::CHOKE] = sendSpecificMessage<MessageType::CHOKE>;
            handlers[MessageType::UNCHOKE] = sendSpecificMessage<MessageType::UNCHOKE>;
            handlers[MessageType::INTERESTED] = sendSpecificMessage<MessageType::INTERESTED>;
            handlers[MessageType::NOTINTERESTED] = sendSpecificMessage<MessageType::NOTINTERESTED>;
            handlers[MessageType::HAVE] = sendSpecificMessage<MessageType::HAVE>;
            handlers[MessageType::BITFIELD] = sendSpecificMessage<MessageType::BITFIELD>;
            handlers[MessageType::REQUEST] = sendSpecificMessage<MessageType::REQUEST>;
            handlers[MessageType::PIECE] = sendSpecificMessage<MessageType::PIECE>;
            handlers[MessageType::CANCEL] = sendSpecificMessage<MessageType::CANCEL>;
        }

		auto itr = handlers.find(msg.GetType());
		if (itr == handlers.end())
		{
			throw UnExpectedMessage();
		}
		itr->second(socket, msg);
    }

    MessageParcel const MessagingSocket::ReceiveMessage(MessageType const msgType) const
    {
        static std::unordered_map<MessageType, std::function<MessageParcel const(ConnectedSocket const&)>> handlers;
        if (handlers.empty())
        {
            handlers[MessageType::CHOKE] = receiveSpecificMessage<MessageType::CHOKE>;
            handlers[MessageType::UNCHOKE] = receiveSpecificMessage<MessageType::UNCHOKE>;
            handlers[MessageType::INTERESTED] = receiveSpecificMessage<MessageType::INTERESTED>;
            handlers[MessageType::NOTINTERESTED] = receiveSpecificMessage<MessageType::NOTINTERESTED>;
            handlers[MessageType::HAVE] = receiveSpecificMessage<MessageType::HAVE>;
            handlers[MessageType::BITFIELD] = receiveSpecificMessage<MessageType::BITFIELD>;
            handlers[MessageType::REQUEST] = receiveSpecificMessage<MessageType::REQUEST>;
            handlers[MessageType::PIECE] = receiveSpecificMessage<MessageType::PIECE>;
            handlers[MessageType::CANCEL] = receiveSpecificMessage<MessageType::CANCEL>;
        }

		auto itr = handlers.find(msgType);
		if (itr == handlers.end())
		{
			throw UnExpectedMessage();
		}
		return itr->second(socket);		
    }
}
