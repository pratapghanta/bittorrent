#include "peer/MessageParcelFactory.hpp"

namespace BT
{
    MessageParcel const MessageParcelFactory::GetChokedMessage()
	{
        MessagePayload empty;
		return MessageParcel(MessageType::CHOKE, 1ul, empty);
	}

	MessageParcel const MessageParcelFactory::GetUnChokedMessage()
	{
        MessagePayload empty;
		return MessageParcel(MessageType::UNCHOKE, 1ul, empty);
	}

	MessageParcel const MessageParcelFactory::GetInterestedMessage()
	{
        MessagePayload empty;
		return MessageParcel(MessageType::INTERESTED, 1ul, empty);
	}

	MessageParcel const MessageParcelFactory::GetNotInterestedMessage()
	{
		MessagePayload empty;
		return MessageParcel(MessageType::NOTINTERESTED, 1ul, empty);
	}

	MessageParcel const MessageParcelFactory::GetHaveMessage(long const have)
	{
        return MessageParcel(MessageType::HAVE, sizeof(have), MessagePayload{have});
	}

	MessageParcel const MessageParcelFactory::GetBitfieldMessage(std::string const& bitfield)
	{
        return MessageParcel(MessageType::BITFIELD, bitfield.length(), MessagePayload{bitfield});
	}

	MessageParcel const MessageParcelFactory::GetRequestMessage(RequestParcel const& request)
	{
        return MessageParcel(MessageType::REQUEST, 
                             1ul + request.Size(), 
                             MessagePayload{request});
	}

	MessageParcel const MessageParcelFactory::GetPieceMessage(PieceParcel const& piece)
	{
        return MessageParcel(MessageType::PIECE, 
                             1ul + piece.Size(), 
                             MessagePayload{piece});
	}

	MessageParcel const MessageParcelFactory::GetCancelMessage(RequestParcel const& cancel)
	{
        return MessageParcel(MessageType::CANCEL, 
                             1ul + cancel.Size(), 
                             MessagePayload{cancel});
	}

	MessageParcel const MessageParcelFactory::GetKeepAliveMessage()
	{
        MessagePayload empty;
		return MessageParcel(MessageType::INTERESTED, 0ul, empty);
	}
}