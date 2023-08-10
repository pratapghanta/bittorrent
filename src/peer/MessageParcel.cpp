#include <cassert>
#include <cstring>

#include "peer/MessageParcel.hpp"

namespace BT
{
	MessageParcel::MessageParcel() 
		: length{0}, type{MessageType::END} {}

	MessageParcel::MessageParcel(MessageType const mt, 
								 MessageLength const len, 
								 MessagePayload const& mp)
		: length{len}, type{mt}, payload{mp} {}
	
	void MessageParcel::reset() 
	{
		// TODO: Take advantage of std::swap with
		// default MessageParcel instance?
		type = MessageType::END;
		length = 0ul;
	}

	std::string const MessageParcel::GetBitfield() const 
	{
		assert(this->IsBitfield());
	    return std::get<std::string>(this->payload);
	}

	Have const MessageParcel::GetHave() const 
	{
		assert(this->IsHave());
		return std::get<Have>(this->payload);
	}

	PieceParcel const MessageParcel::GetPiece() const 
	{
		assert(this->IsPiece());
		return std::get<PieceParcel>(this->payload);
	}

	RequestParcel const MessageParcel::GetRequest() const 
	{
		assert(this->IsRequest());
		return std::get<RequestParcel>(this->payload);
	}

	RequestParcel const MessageParcel::GetCancel() const 
	{
		assert(this->IsCancel());
	    return std::get<RequestParcel>(this->payload);
	}
}