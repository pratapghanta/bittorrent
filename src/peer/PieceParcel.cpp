#include <cstring>

#include "peer/MessageParcel.hpp"

BT::PieceParcel::PieceParcel() : index(0), begin(0), piece(nullptr) {}

BT::PieceParcel::PieceParcel(long const i, long const b, char const * const p) : index(i), begin(b), piece(nullptr) {
	if (p == nullptr)
		return;

	piece = new char[strlen(p)];
	strcpy(piece, p);
}

BT::PieceParcel::PieceParcel(BT::PieceParcel const& other) : index(other.index), begin(other.begin), piece(nullptr) {
	if (other.piece == nullptr)
		return;

	piece = new char[strlen(other.piece)];
	strcpy(piece, other.piece);
}

BT::PieceParcel::PieceParcel(BT::PieceParcel&& other) : index(other.index), begin(other.begin), piece(other.piece) {
	other.reset();
}

BT::PieceParcel& BT::PieceParcel::operator=(BT::PieceParcel other) {
	BT::swap(*this, other);
	return *this;
}

void BT::PieceParcel::reset() {
	index = 0;
	begin = 0;
	piece = nullptr;
}

void BT::swap(BT::PieceParcel& a, BT::PieceParcel& b) {
	std::swap(a.begin, b.begin);
	std::swap(a.index, b.index);
	std::swap(a.piece, b.piece);
}

bool BT::operator==(BT::PieceParcel const& a, BT::PieceParcel const& b) {
	if (!((a.index == b.index) && (a.begin == b.begin)))
		return false;

	if (a.piece == nullptr)
		return (b.piece == nullptr);
	return std::string(a.piece) == std::string(b.piece);
}

BT::PieceParcel::~PieceParcel() {
	if (piece == nullptr)
		return;

	delete[] piece;
	piece = nullptr;
}
