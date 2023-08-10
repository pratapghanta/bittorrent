#include <cstring>
#include <utility>

#include "peer/PieceParcel.hpp"

namespace BT
{
	PieceParcel::PieceParcel() 
		: index(0), begin(0), piece(nullptr) 
	{}

	PieceParcel::PieceParcel(uint32_t const i, uint32_t const b, char const * const p) 
		: index(i), begin(b), piece(nullptr) 
	{
		if (p == nullptr)
		{
			return;
		}

		piece = new char[strlen(p)];
		strcpy(piece, p);
	}

	PieceParcel::PieceParcel(PieceParcel const& other) 
		: index(other.index), begin(other.begin), piece(nullptr) 
	{
		if (other.piece == nullptr)
		{
			return;
		}

		piece = new char[strlen(other.piece)];
		strcpy(piece, other.piece);
	}

	PieceParcel::PieceParcel(PieceParcel&& other) 
		: index(other.index), begin(other.begin), piece(other.piece) 
	{
		other.piece = nullptr;
	}

	PieceParcel& PieceParcel::operator=(PieceParcel other) 
	{
		swap(*this, other);
		return *this;
	}

	void swap(PieceParcel& a, PieceParcel& b) 
	{
		std::swap(a.begin, b.begin);
		std::swap(a.index, b.index);
		std::swap(a.piece, b.piece);
	}

	bool operator==(PieceParcel const& a, PieceParcel const& b) 
	{
		if (!((a.index == b.index) && (a.begin == b.begin)))
		{
			return false;
		}

		if (a.piece == nullptr)
		{
			return (b.piece == nullptr);
		}

		return strcmp(a.piece, b.piece) == 0;
	}

	PieceParcel::~PieceParcel() 
	{
		if (piece != nullptr)
		{
			delete[] piece;
			piece = nullptr;
		}
	}

	unsigned int PieceParcel::Size() const
	{
		unsigned int pieceLength = (piece == nullptr) ? 0 : strlen(piece);
		return sizeof(index) + sizeof(begin) + pieceLength; 
	}
}