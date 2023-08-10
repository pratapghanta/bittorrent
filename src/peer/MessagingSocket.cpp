#include <cassert>
#include <cstring>
#include <functional>
#include <unordered_map>
#include <vector>

#include "common/Defaults.hpp"
#include "common/Errors.hpp"
#include "common/Helpers.hpp"
#include "peer/ConnectedSocketParcel.hpp"
#include "peer/ConnectedSocket.hpp"
#include "peer/MessageParcel.hpp"
#include "peer/MessageParcelFactory.hpp"
#include "peer/MessagingSocket.hpp"
#include "socket/ConnectedSocket.hpp"

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
		template<typename T>
		void receiveIntegerData(ConnectedSocketPtr const& connectedSocketPtr, T& value) 
		{
			value = 0;
			connectedSocketPtr->Receive((void*)&value, sizeof(value));
		}

		MessageLength const receiveMessageLength(ConnectedSocketPtr const& connectedSocketPtr) 
		{
			MessageLength msgLength = 0;
			receiveIntegerData(connectedSocketPtr, msgLength);
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
		                                           MessageLength const) 
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
		                                                              MessageLength const) 
		{
			Have have = 0;
			receiveIntegerData(connectedSocketPtr, have);
			return MessageParcelFactory::GetHaveMessage(have);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::BITFIELD>(ConnectedSocketPtr const& connectedSocketPtr,
		                                                                  MessageLength const totalMsgLength) 
		{
			if (totalMsgLength <= 1)
			{
				throw UnExpectedMessage();
				// TODO: Abort the peer connection as it may be rogue.
			}

			auto const bitfieldLength = totalMsgLength-1;
			std::vector<char> buffer(bitfieldLength+1, '\0');
			connectedSocketPtr->Receive((void*)&(buffer[0]), bitfieldLength);
			return  MessageParcelFactory::GetBitfieldMessage(&(buffer[0]));
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::REQUEST>(ConnectedSocketPtr const& connectedSocketPtr,
		                                                                 MessageLength const) 
		{
			RequestParcel request;
			receiveIntegerData(connectedSocketPtr, request.index);
			receiveIntegerData(connectedSocketPtr, request.begin); 
			receiveIntegerData(connectedSocketPtr, request.length);
			return  MessageParcelFactory::GetRequestMessage(request);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::PIECE>(ConnectedSocketPtr const& connectedSocketPtr,
		                                                               MessageLength const totalMsgLength) 
		{
			PieceParcel pieceParcel;
			
			receiveIntegerData(connectedSocketPtr, pieceParcel.index);
			receiveIntegerData(connectedSocketPtr, pieceParcel.begin); 
			
			auto const pieceLength = totalMsgLength - 1 - pieceParcel.Size();
			if (pieceLength > 0)
			{
				pieceParcel.piece = new char[pieceLength+1];
				connectedSocketPtr->Receive((void*)pieceParcel.piece, pieceLength);
				pieceParcel.piece[pieceLength] = '\0';
			}
			
			return  MessageParcelFactory::GetPieceMessage(pieceParcel);
		}

		template<>
		MessageParcel const receiveSpecificMessage<MessageType::CANCEL>(ConnectedSocketPtr const& connectedSocketPtr,
		                                                                MessageLength const) 
		{
			RequestParcel cancel;
			receiveIntegerData(connectedSocketPtr, cancel.index);
			receiveIntegerData(connectedSocketPtr, cancel.begin); 
			receiveIntegerData(connectedSocketPtr, cancel.length);
			return  MessageParcelFactory::GetCancelMessage(cancel);
		}

		template<typename T>
		void sendIntegerData(ConnectedSocketPtr const& connectedSocketPtr, T const value) 
		{
			connectedSocketPtr->Send((void*)&value, sizeof(value));
		}

		void sendMessageLength(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			MessageLength msgLength = msg.GetLength();
			sendIntegerData(connectedSocketPtr, msgLength);
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
			sendIntegerData(connectedSocketPtr, msg.GetHave());
		}

		template<>
		void sendSpecificMessage<MessageType::BITFIELD>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			/* bitfield message can be skipped when the peer has NO pieces */
			auto bitfield = msg.GetBitfield();
			if (bitfield.find('1') == std::string::npos)
			{
				return;
			}

			sendMessageAttributes(connectedSocketPtr, msg);
			connectedSocketPtr->Send((void*)bitfield.c_str(), bitfield.length());
		}

		template<>
		void sendSpecificMessage<MessageType::REQUEST>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocketPtr, msg);

			auto request = msg.GetRequest();
			sendIntegerData(connectedSocketPtr, request.index);
			sendIntegerData(connectedSocketPtr, request.begin);
			sendIntegerData(connectedSocketPtr, request.length);
		}

		template<>
		void sendSpecificMessage<MessageType::PIECE>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocketPtr, msg);

			PieceParcel pieceParcel = msg.GetPiece();
			sendIntegerData(connectedSocketPtr, pieceParcel.index);
			sendIntegerData(connectedSocketPtr, pieceParcel.begin);
			if (pieceParcel.piece != nullptr)
			{
				connectedSocketPtr->Send((void*)pieceParcel.piece, strlen(pieceParcel.piece));
			}
		}

		template<>
		void sendSpecificMessage<MessageType::CANCEL>(ConnectedSocketPtr const& connectedSocketPtr, MessageParcel const &msg) 
		{
			sendMessageAttributes(connectedSocketPtr, msg);

			auto cancel = msg.GetRequest();
			sendIntegerData(connectedSocketPtr, cancel.index);
			sendIntegerData(connectedSocketPtr, cancel.begin);
			sendIntegerData(connectedSocketPtr, cancel.length);
		}
	}
}

namespace BT 
{
    MessagingSocket::MessagingSocket(ConnectedSocketParcel const& parcel)
        : connectedSocketPtr(nullptr) // xpragha
    {
        fromId = ""; // TODO: CalculateId(parcel.fromIp, parcel.fromPort);
        toId = CalculateId(parcel.toIp, parcel.toPort);
		connectedSocketPtr = std::make_unique<ConnectedSocket>(parcel);
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

		MessageLength msgLength = receiveMessageLength(connectedSocketPtr);
		if (msgLength == 0)
		{
			return MessageParcelFactory::GetKeepAliveMessage();
		}

		MessageType msgType = receiveMessageType(connectedSocketPtr);
		auto itr = handlers.find(msgType);
		if (itr == handlers.end())
		{
			throw UnExpectedMessage();
		}
		return itr->second(connectedSocketPtr, msgLength);		
    }
}
