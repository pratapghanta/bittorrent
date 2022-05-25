#include <cassert>
#include <cstring>

#include "peer/Message.hpp"

bool BT::operator==(BT::Request_t const& a, BT::Request_t const& b) {
	return (a.index == b.index) &&
		(a.begin == b.begin) &&
		(a.length == b.length);
}

BT::Message_t::data::data() {
	memset(static_cast<void*>(this), 0, sizeof(BT::Message_t::data));
}

BT::Message_t::data::~data() {}

void BT::Message_t::reset() {
	type = MessageType::END;
	length = 0ul;
	memset(static_cast<void*>(&payload), 0, sizeof(payload));
}

BT::Message_t::Message_t(BT::Message_t const& other) 
	: type(other.getType()), length(other.getLength()) {
	memcpy(&payload, &(other.payload), sizeof(payload));
}

BT::Message_t::Message_t(BT::Message_t&& other)
	: type(other.getType()), length(other.getLength()) {
	memcpy(&payload, &(other.payload), sizeof(payload));
	other.reset(); /* TODO: Why is move costlier than copy ? */
}

BT::Message_t& BT::Message_t::operator=(BT::Message_t const &other) {
	type = other.getType();
	length = other.getLength();
	memcpy(&payload, &(other.payload), sizeof(payload));

	return *this;
}

BT::Message_t& BT::Message_t::operator=(BT::Message_t &&other) {
	type = other.getType();
	length = other.getLength();
	memcpy(&payload, &(other.payload), sizeof(payload));

	other.reset();

	return *this;
}

BT::Message_t const BT::Message_t::getChokedMessage() {
	BT::Message_t msg;

	msg.type = MessageType::CHOKE;
	msg.length = 1ul;

	return msg;
}

BT::Message_t const BT::Message_t::getUnChokedMessage() {
	BT::Message_t msg;

	msg.type = MessageType::UNCHOKE;
	msg.length = 1ul;

	return msg;
}

BT::Message_t const BT::Message_t::getInterestedMessage() {
	BT::Message_t msg;

	msg.type = MessageType::INTERESTED;
	msg.length = 1ul;

	return msg;
}

BT::Message_t const BT::Message_t::getNotInterestedMessage() {
	BT::Message_t msg;

	msg.type = MessageType::NOTINTERESTED;
	msg.length = 1ul;

	return msg;
}

BT::Message_t const BT::Message_t::getHaveMessage(long const have) {
	BT::Message_t msg;

	msg.type = MessageType::HAVE;
	msg.length = sizeof(have);
	msg.payload.have = have;

	return msg;
}

BT::Message_t const BT::Message_t::getBitfieldMessage(std::string const& bitfield) {
	BT::Message_t msg;

	msg.type = MessageType::BITFIELD;
	msg.length = bitfield.length();
	msg.payload.bitfield = bitfield;

	return msg;
}

BT::Message_t const BT::Message_t::getRequestMessage(BT::Request_t const& request) {
	BT::Message_t msg;

	msg.type = MessageType::REQUEST;
	msg.length = 1ul + sizeof(request.index) + sizeof(request.begin) + sizeof(request.length);
	msg.payload.request = request;

	return msg;
}

BT::Message_t const BT::Message_t::getPieceMessage(BT::Piece_t const& piece) {
	BT::Message_t msg;

	msg.type = MessageType::PIECE;
	msg.length = 1ul + sizeof(piece.index) + sizeof(piece.begin);
	msg.payload.piece = piece;

	return msg;
}

BT::Message_t const BT::Message_t::getCancelMessage(BT::Request_t const& cancel) {
	BT::Message_t msg;

	msg.type = MessageType::CANCEL;
	msg.length = 1ul + sizeof(cancel.index) + sizeof(cancel.begin) + sizeof(cancel.length);
	msg.payload.cancel = cancel;

	return msg;
}

BT::Message_t const BT::Message_t::getKeepAliveMessage() {
	Message_t msg;

	msg.type = MessageType::INTERESTED;
	msg.length = 0ul;

	return msg;
}

std::string const BT::Message_t::getBitfield() const {
	assert(this->isBitfield());
    return this->payload.bitfield;
}

long const BT::Message_t::getHave() const {
	assert(this->isHave());
	return this->payload.have;;
}

BT::Piece_t const BT::Message_t::getPiece() const {
	assert(this->isPiece());

	BT::Piece_t out;
    out.index = this->payload.piece.index;
    out.begin = this->payload.piece.begin;
    if(this->payload.piece.piece != nullptr) {
        out.piece = new char[strlen(this->payload.piece.piece)];
        strcpy(out.piece, this->payload.piece.piece);
    }

	return out;
}

BT::Request_t const BT::Message_t::getRequest() const {
	assert(this->isRequest());
	return this->payload.request;
}

BT::Request_t const BT::Message_t::getCancel() const {
	assert(this->isCancel());
    return this->payload.cancel;
}

BT::Message_t::~Message_t() {
	/* EVIL EVIL EVIL destructor */
	switch (this->type) {
	case BT::Message_t::MessageType::CHOKE:
	case BT::Message_t::MessageType::UNCHOKE:
	case BT::Message_t::MessageType::INTERESTED:
	case BT::Message_t::MessageType::NOTINTERESTED:
	case BT::Message_t::MessageType::BITFIELD:
		// this->payload.bitfield.~string();
		break;
	case BT::Message_t::MessageType::PIECE:
		this->payload.piece.~Piece_t();
		break;
	default:
		break;
	}
}

