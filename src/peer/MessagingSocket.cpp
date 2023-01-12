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
		long const receiveLong(ConnectedSocket const& connectedSocket) 
		{
			long value = 0;
			connectedSocket.Receive((void*)&value, sizeof(value));
			return value;
		}

		unsigned long const receiveMessageLength(ConnectedSocket const& connectedSocket) 
		{
			unsigned long msgLength = 0;
			connectedSocket.Receive((void*)&msgLength, sizeof(msgLength));
			return msgLength;
		}

		MessageType const receiveMessageType(ConnectedSocket const& connectedSocket) 
		{
			MessageType msgType = MessageType::END;
			connectedSocket.Receive((void*)&msgType, sizeof(msgType));
			return msgType;
		}

		template<MessageType msgType>
		MessageParcel const receiveSpecificMessage(ConnectedSocket const&,
		                                           unsigned long const) 
		{
			std::unordered_map<MessageType, std::function<MessageParcel const()>> handlers;
			handlers[MessageType::CHOKE] = MessageParcelFactory::GetChokedMessage;
			handlers[MessageType::UNCHOKE] = MessageParcelFactory::GetUnChokedMessage;
			handlers[MessageType::INTERESTED] = MessageParcelFactory::GetInterestedMessage;
			handlers[MessageType::NOTINTERESTED] = MessageParcelFactory::GetNotInterestedMessage;

			auto itr = handlers.find(msgType);
			if (itr == handlers.end())
			{ 
				throw UnExpectedMessage();
			}

			return itr->second();
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::HAVE>(ConnectedSocket const& connectedSocket,
		                                                              unsigned long const) 
		{
			long have = receiveLong(connectedSocket);
			return MessageParcelFactory::GetHaveMessage(have);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::BITFIELD>(ConnectedSocket const& connectedSocket,
		                                                                  unsigned long const msgLength) 
		{
			char buffer[Defaults::MaxBufferSize] = "";
			connectedSocket.Receive((void*)&buffer, msgLength-1);
			buffer[msgLength-1] = '\0';
			return  MessageParcelFactory::GetBitfieldMessage(buffer);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::REQUEST>(ConnectedSocket const& connectedSocket,
		                                                                 unsigned long const) 
		{
			RequestParcel request(receiveLong(connectedSocket), 
			                      receiveLong(connectedSocket), 
								  receiveLong(connectedSocket));
			return  MessageParcelFactory::GetRequestMessage(request);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::PIECE>(ConnectedSocket const& connectedSocket,
		                                                               unsigned long const) 
		{
			PieceParcel piece(receiveLong(connectedSocket), 
			                  receiveLong(connectedSocket), nullptr);
			return  MessageParcelFactory::GetPieceMessage(piece);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::CANCEL>(ConnectedSocket const& connectedSocket,
		                                                                unsigned long const) 
		{
			RequestParcel cancel(receiveLong(connectedSocket),
			                     receiveLong(connectedSocket),
								 receiveLong(connectedSocket));
			return  MessageParcelFactory::GetCancelMessage(cancel);
		}

		void sendLong(ConnectedSocket const& connectedSocket, long const value) 
		{
			connectedSocket.Send((void*)&value, sizeof(value));
		}

		void sendMessageLength(ConnectedSocket const& connectedSocket, MessageParcel const &msg) 
		{
			unsigned long msgLength = msg.GetLength();
			connectedSocket.Send((void*)&msgLength, sizeof(msgLength));
		}

		void sendMessageType(ConnectedSocket const& connectedSocket, MessageParcel const &msg) 
		{
			MessageType type = msg.GetType();
			connectedSocket.Send((void*)&type, sizeof(type));
		}

		void sendMessageAttributes(ConnectedSocket const& connectedSocket, MessageParcel const &msg) 
		{
			sendMessageLength(connectedSocket, msg);
			sendMessageType(connectedSocket, msg);
		}

		template<MessageType msgType>
		void sendSpecificMessage(ConnectedSocket const& connectedSocket, MessageParcel const &msg) {}

		template<>
		void sendSpecificMessage<MessageType::CHOKE>(ConnectedSocket const& connectedSocket, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocket, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::UNCHOKE>(ConnectedSocket const& connectedSocket, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocket, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::INTERESTED>(ConnectedSocket const& connectedSocket, MessageParcel const &msg) 
		{
			sendMessageLength(connectedSocket, msg);
			if (msg.GetLength() == 0) 
			{
				return; /* Keepalive */
			}
			sendMessageType(connectedSocket, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::NOTINTERESTED>(ConnectedSocket const& connectedSocket, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocket, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::HAVE>(ConnectedSocket const& connectedSocket, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocket, msg);
			sendLong(connectedSocket, msg.GetHave());
		}

		template<>
		void sendSpecificMessage<MessageType::BITFIELD>(ConnectedSocket const& connectedSocket, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocket, msg);

			auto bitfield = msg.GetBitfield();
			connectedSocket.Send((void*)bitfield.c_str(), bitfield.length());
		}

		template<>
		void sendSpecificMessage<MessageType::REQUEST>(ConnectedSocket const& connectedSocket, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocket, msg);

			auto request = msg.GetRequest();
			sendLong(connectedSocket, request.index);
			sendLong(connectedSocket, request.begin);
			sendLong(connectedSocket, request.length);
		}

		template<>
		void sendSpecificMessage<MessageType::PIECE>(ConnectedSocket const& connectedSocket, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocket, msg);

			auto piece = msg.GetPiece();
			sendLong(connectedSocket, piece.index);
			sendLong(connectedSocket, piece.begin);
		}

		template<>
		void sendSpecificMessage<MessageType::CANCEL>(ConnectedSocket const& connectedSocket, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocket, msg);

			auto cancel = msg.GetRequest();
			sendLong(connectedSocket, cancel.index);
			sendLong(connectedSocket, cancel.begin);
			sendLong(connectedSocket, cancel.length);
		}
	}
}

namespace BT 
{
    MessagingSocket::MessagingSocket(ConnectedSocketParcel const& parcel)
        : connectedSocket(parcel)
    {
        fromId = CalculateId(parcel.fromIp, parcel.fromPort);
        toId = CalculateId(parcel.toIp, parcel.toPort);
    }

    int MessagingSocket::Send(char const * const buffer, unsigned int count) const
    {
        return connectedSocket.Send(buffer, count);
    }

    int MessagingSocket::Receive(char* buffer, unsigned int count) const
    {
        return connectedSocket.Receive(buffer, count);
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
		itr->second(connectedSocket, msg);
    }

    MessageParcel const MessagingSocket::ReceiveMessage() const
    {
        static std::unordered_map<MessageType, 
		                          std::function<MessageParcel const(ConnectedSocket const&, unsigned long const)>> handlers;
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

		unsigned long msgLength = receiveMessageLength(connectedSocket);
		MessageType msgType = receiveMessageType(connectedSocket);
		
		auto itr = handlers.find(msgType);
		if (itr == handlers.end())
		{
			throw UnExpectedMessage();
		}
		return itr->second(connectedSocket, msgLength);		
    }
}
