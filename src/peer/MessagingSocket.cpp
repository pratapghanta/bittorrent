#include <cstring>
#include <functional>
#include <unordered_map>

#include "common/Defaults.hpp"
#include "common/Errors.hpp"
#include "common/Helpers.hpp"
#include "peer/ConnectedSocketParcel.hpp"
#include "peer/ConnectedSocket.hpp"
#include "peer/MessageParcel.hpp"
#include "peer/MessageParcelFactory.hpp"
#include "peer/MessagingSocket.hpp"

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
		long const receiveLong(ConnectedSocketPtr const& connectedSocketPtr) 
		{
			long value = 0;
			connectedSocketPtr->Receive((void*)&value, sizeof(value));
			return value;
		}

		unsigned long const receiveMessageLength(ConnectedSocketPtr const& connectedSocketPtr) 
		{
			unsigned long msgLength = 0;
			connectedSocketPtr->Receive((void*)&msgLength, sizeof(msgLength));
			return msgLength;
		}

		MessageType const receiveMessageType(ConnectedSocketPtr const& connectedSocketPtr) 
		{
			MessageType msgType = MessageType::END;
			connectedSocketPtr->Receive((void*)&msgType, sizeof(msgType));
			return msgType;
		}

		template<MessageType msgType>
		MessageParcel const receiveSpecificMessage(ConnectedSocketPtr const&,
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
		MessageParcel const receiveSpecificMessage<MessageType::HAVE>(ConnectedSocketPtr const& connectedSocketPtr,
		                                                              unsigned long const) 
		{
			long have = receiveLong(connectedSocketPtr);
			return MessageParcelFactory::GetHaveMessage(have);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::BITFIELD>(ConnectedSocketPtr const& connectedSocketPtr,
		                                                                  unsigned long const msgLength) 
		{
			char buffer[Defaults::MaxBufferSize] = "";
			connectedSocketPtr->Receive((void*)&buffer, msgLength-1);
			buffer[msgLength-1] = '\0';
			return  MessageParcelFactory::GetBitfieldMessage(buffer);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::REQUEST>(ConnectedSocketPtr const& connectedSocketPtr,
		                                                                 unsigned long const) 
		{
			RequestParcel request(receiveLong(connectedSocketPtr), 
			                      receiveLong(connectedSocketPtr), 
								  receiveLong(connectedSocketPtr));
			return  MessageParcelFactory::GetRequestMessage(request);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::PIECE>(ConnectedSocketPtr const& connectedSocketPtr,
		                                                               unsigned long const) 
		{
			PieceParcel piece(receiveLong(connectedSocketPtr), 
			                  receiveLong(connectedSocketPtr), nullptr);
			return  MessageParcelFactory::GetPieceMessage(piece);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::CANCEL>(ConnectedSocketPtr const& connectedSocketPtr,
		                                                                unsigned long const) 
		{
			RequestParcel cancel(receiveLong(connectedSocketPtr),
			                     receiveLong(connectedSocketPtr),
								 receiveLong(connectedSocketPtr));
			return  MessageParcelFactory::GetCancelMessage(cancel);
		}

		void sendLong(ConnectedSocketPtr const& connectedSocketPtr, long const value) 
		{
			connectedSocketPtr->Send((void*)&value, sizeof(value));
		}

		void sendMessageLength(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			unsigned long msgLength = msg.GetLength();
			connectedSocketPtr->Send((void*)&msgLength, sizeof(msgLength));
		}

		void sendMessageType(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			MessageType type = msg.GetType();
			connectedSocketPtr->Send((void*)&type, sizeof(type));
		}

		void sendMessageAttributes(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageLength(connectedSocketPtr, msg);
			sendMessageType(connectedSocketPtr, msg);
		}

		template<MessageType msgType>
		void sendSpecificMessage(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) {}

		template<>
		void sendSpecificMessage<MessageType::CHOKE>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocketPtr, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::UNCHOKE>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocketPtr, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::INTERESTED>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageLength(connectedSocketPtr, msg);
			if (msg.GetLength() == 0) 
			{
				return; /* Keepalive */
			}
			sendMessageType(connectedSocketPtr, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::NOTINTERESTED>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocketPtr, msg);
		}

		template<>
		void sendSpecificMessage<MessageType::HAVE>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocketPtr, msg);
			sendLong(connectedSocketPtr, msg.GetHave());
		}

		template<>
		void sendSpecificMessage<MessageType::BITFIELD>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocketPtr, msg);

			auto bitfield = msg.GetBitfield();
			connectedSocketPtr->Send((void*)bitfield.c_str(), bitfield.length());
		}

		template<>
		void sendSpecificMessage<MessageType::REQUEST>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocketPtr, msg);

			auto request = msg.GetRequest();
			sendLong(connectedSocketPtr, request.index);
			sendLong(connectedSocketPtr, request.begin);
			sendLong(connectedSocketPtr, request.length);
		}

		template<>
		void sendSpecificMessage<MessageType::PIECE>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocketPtr, msg);

			auto piece = msg.GetPiece();
			sendLong(connectedSocketPtr, piece.index);
			sendLong(connectedSocketPtr, piece.begin);
		}

		template<>
		void sendSpecificMessage<MessageType::CANCEL>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocketPtr, msg);

			auto cancel = msg.GetRequest();
			sendLong(connectedSocketPtr, cancel.index);
			sendLong(connectedSocketPtr, cancel.begin);
			sendLong(connectedSocketPtr, cancel.length);
		}
	}
}

namespace BT 
{
    MessagingSocket::MessagingSocket(ConnectedSocketParcel const& parcel)
        : connectedSocketPtr(nullptr) // xpragha
    {
        fromId = CalculateId(parcel.fromIp, parcel.fromPort);
        toId = CalculateId(parcel.toIp, parcel.toPort);
    }

    unsigned int MessagingSocket::Send(char const * const buffer, unsigned int count) const
    {
        return connectedSocketPtr->Send(buffer, count);
    }

    unsigned int MessagingSocket::Receive(char* buffer, unsigned int count) const
    {
        return connectedSocketPtr->Receive(buffer, count);
    }

    void MessagingSocket::SendMessage(MessageParcel const& msg) const
    {
        static std::unordered_map<MessageType, std::function<void(ConnectedSocketPtr const&, MessageParcel const&)>> handlers;
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
		itr->second(connectedSocketPtr, msg);
    }

    MessageParcel const MessagingSocket::ReceiveMessage() const
    {
        static std::unordered_map<MessageType, 
		                          std::function<MessageParcel const(ConnectedSocketPtr const&, unsigned long const)>> handlers;
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

		unsigned long msgLength = receiveMessageLength(connectedSocketPtr);
		MessageType msgType = receiveMessageType(connectedSocketPtr);
		
		auto itr = handlers.find(msgType);
		if (itr == handlers.end())
		{
			throw UnExpectedMessage();
		}
		return itr->second(connectedSocketPtr, msgLength);		
    }
}
