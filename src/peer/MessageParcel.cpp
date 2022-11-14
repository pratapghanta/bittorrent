#include <cassert>
#include <cstring>

#include "peer/MessageParcel.hpp"

namespace BT
{
	void MessageParcel::reset() 
	{
		// TODO: Take advantage of std::swap with
		// default MessageParcel instance?
		type = MessageType::END;
		length = 0ul;
	}

	MessageParcel const MessageParcel::getChokedMessage()
	{
		MessageParcel msg;

		msg.type = MessageType::CHOKE;
		msg.length = 1ul;

		return msg;
	}

	MessageParcel const MessageParcel::getUnChokedMessage()
	{
		MessageParcel msg;

		msg.type = MessageType::UNCHOKE;
		msg.length = 1ul;

		return msg;
	}

	MessageParcel const MessageParcel::getInterestedMessage() {
		MessageParcel msg;

		msg.type = MessageType::INTERESTED;
		msg.length = 1ul;

		return msg;
	}

	MessageParcel const MessageParcel::getNotInterestedMessage() {
		MessageParcel msg;

		msg.type = MessageType::NOTINTERESTED;
		msg.length = 1ul;

		return msg;
	}

	MessageParcel const MessageParcel::getHaveMessage(long const have) {
		MessageParcel msg;

		msg.type = MessageType::HAVE;
		msg.length = sizeof(have);
		msg.payload.have = have;

		return msg;
	}

	MessageParcel const MessageParcel::getBitfieldMessage(std::string const& bitfield) {
		MessageParcel msg;

		msg.type = MessageType::BITFIELD;
		msg.length = bitfield.length();
		msg.payload.bitfield = bitfield;

		return msg;
	}

	MessageParcel const MessageParcel::getRequestMessage(RequestParcel const& request) {
		MessageParcel msg;

		msg.type = MessageType::REQUEST;
		msg.length = 1ul + sizeof(request.index) + sizeof(request.begin) + sizeof(request.length);
		msg.payload.request = request;

		return msg;
	}

	MessageParcel const MessageParcel::getPieceMessage(PieceParcel const& piece) {
		MessageParcel msg;

		msg.type = MessageType::PIECE;
		msg.length = 1ul + sizeof(piece.index) + sizeof(piece.begin);
		msg.payload.piece = piece;

		return msg;
	}

	MessageParcel const MessageParcel::getCancelMessage(RequestParcel const& cancel) {
		MessageParcel msg;

		msg.type = MessageType::CANCEL;
		msg.length = 1ul + sizeof(cancel.index) + sizeof(cancel.begin) + sizeof(cancel.length);
		msg.payload.cancel = cancel;

		return msg;
	}

	MessageParcel const MessageParcel::getKeepAliveMessage() {
		MessageParcel msg;

		msg.type = MessageType::INTERESTED;
		msg.length = 0ul;

		return msg;
	}

	std::string const MessageParcel::getBitfield() const {
		assert(this->isBitfield());
	    return this->payload.bitfield;
	}

	long const MessageParcel::getHave() const {
		assert(this->isHave());
		return this->payload.have;;
	}

	PieceParcel const MessageParcel::getPiece() const {
		assert(this->isPiece());

		PieceParcel out;
	    out.index = this->payload.piece.index;
	    out.begin = this->payload.piece.begin;
	    if(this->payload.piece.piece != nullptr) {
	        out.piece = new char[strlen(this->payload.piece.piece)];
	        strcpy(out.piece, this->payload.piece.piece);
	    }

		return out;
	}

	RequestParcel const MessageParcel::getRequest() const {
		assert(this->isRequest());
		return this->payload.request;
	}

	RequestParcel const MessageParcel::getCancel() const {
		assert(this->isCancel());
	    return this->payload.cancel;
	}

	MessageParcel::~MessageParcel() {
		/* EVIL EVIL EVIL destructor */
		switch (this->type) {
		case MessageParcel::MessageType::CHOKE:
		case MessageParcel::MessageType::UNCHOKE:
		case MessageParcel::MessageType::INTERESTED:
		case MessageParcel::MessageType::NOTINTERESTED:
		case MessageParcel::MessageType::BITFIELD:
			// this->payload.bitfield.~string();
			break;
		case MessageParcel::MessageType::PIECE:
			this->payload.piece.~PieceParcel();
			break;
		default:
			break;
		}
	}

}
