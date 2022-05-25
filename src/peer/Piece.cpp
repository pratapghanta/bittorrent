#include <cstring>

#include "peer/Message.hpp"

BT::Piece_t::Piece_t() : index(0), begin(0), piece(nullptr) {}

BT::Piece_t::Piece_t(long const i, long const b, char const * const p) : index(i), begin(b), piece(nullptr) {
	if (p == nullptr)
		return;

	piece = new char[strlen(p)];
	strcpy(piece, p);
}

BT::Piece_t::Piece_t(BT::Piece_t const& other) : index(other.index), begin(other.begin), piece(nullptr) {
	if (other.piece == nullptr)
		return;

	piece = new char[strlen(other.piece)];
	strcpy(piece, other.piece);
}

BT::Piece_t::Piece_t(BT::Piece_t&& other) : index(other.index), begin(other.begin), piece(other.piece) {
	other.reset();
}

BT::Piece_t& BT::Piece_t::operator=(BT::Piece_t other) {
	BT::swap(*this, other);
	return *this;
}

void BT::Piece_t::reset() {
	index = 0;
	begin = 0;
	piece = nullptr;
}

void BT::swap(BT::Piece_t& a, BT::Piece_t& b) {
	std::swap(a.begin, b.begin);
	std::swap(a.index, b.index);
	std::swap(a.piece, b.piece);
}

bool BT::operator==(BT::Piece_t const& a, BT::Piece_t const& b) {
	if (!((a.index == b.index) && (a.begin == b.begin)))
		return false;

	if (a.piece == nullptr)
		return (b.piece == nullptr);
	return std::string(a.piece) == std::string(b.piece);
}

BT::Piece_t::~Piece_t() {
	if (piece == nullptr)
		return;

	delete[] piece;
	piece = nullptr;
}
