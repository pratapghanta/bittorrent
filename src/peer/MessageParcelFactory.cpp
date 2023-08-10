#include "peer/MessageParcelFactory.hpp"

namespace BT
{
    MessageParcel const MessageParcelFactory::GetChokedMessage()
	{
        MessagePayload empty;
		return MessageParcel(MessageType::CHOKE, 1, empty);
	}

	MessageParcel const MessageParcelFactory::GetUnChokedMessage()
	{
        MessagePayload empty;
		return MessageParcel(MessageType::UNCHOKE, 1, empty);
	}

	MessageParcel const MessageParcelFactory::GetInterestedMessage()
	{
        MessagePayload empty;
		return MessageParcel(MessageType::INTERESTED, 1, empty);
	}

	MessageParcel const MessageParcelFactory::GetNotInterestedMessage()
	{
		MessagePayload empty;
		return MessageParcel(MessageType::NOTINTERESTED, 1, empty);
	}

	MessageParcel const MessageParcelFactory::GetHaveMessage(Have const have)
	{
        return MessageParcel(MessageType::HAVE, 
		                     1 + sizeof(have), 
							 MessagePayload{have});
	}

	MessageParcel const MessageParcelFactory::GetBitfieldMessage(std::string const& bitfield)
	{
        return MessageParcel(MessageType::BITFIELD, 
		                     1 + bitfield.length(), 
							 MessagePayload{bitfield});
	}

	MessageParcel const MessageParcelFactory::GetRequestMessage(RequestParcel const& request)
	{
        return MessageParcel(MessageType::REQUEST, 
                             1 + request.Size(), 
                             MessagePayload{request});
	}

	MessageParcel const MessageParcelFactory::GetPieceMessage(PieceParcel const& piece)
	{
        return MessageParcel(MessageType::PIECE, 
                             1 + piece.Size(), 
                             MessagePayload{piece});
	}

	MessageParcel const MessageParcelFactory::GetCancelMessage(RequestParcel const& cancel)
	{
        return MessageParcel(MessageType::CANCEL, 
                             1 + cancel.Size(), 
                             MessagePayload{cancel});
	}

	MessageParcel const MessageParcelFactory::GetKeepAliveMessage()
	{
        MessagePayload empty;
		return MessageParcel(MessageType::INTERESTED, 0, empty);
	}
}