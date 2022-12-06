#include <cassert>
#include <cstring>

#include "peer/MessageParcel.hpp"

namespace BT
{
	MessageParcel::MessageParcel() 
		: type{MessageType::END}, length{0ul} {}

	MessageParcel::MessageParcel(MessageType const mt, 
								unsigned long const len, 
								MessagePayload const& mp)
		: type{mt}, length{len}, payload{mp} {}
	
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

	long const MessageParcel::GetHave() const 
	{
		assert(this->IsHave());
		return std::get<long>(this->payload);
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