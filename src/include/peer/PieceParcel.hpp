#if !defined(PIECE_PARCEL_HPP)
#define PIECE_PARCEL_HPP

#include <cstdint>

namespace BT
{
    /* A message used to transfer data */
    /* The transfer of a piece of file happens block by block */
    struct PieceParcel
    {
        PieceParcel();
        PieceParcel(uint32_t const, uint32_t const, char const * const);
        PieceParcel(PieceParcel const&);
        PieceParcel(PieceParcel&&);
        PieceParcel& operator=(PieceParcel);
        ~PieceParcel();

        unsigned int Size() const;

    public:
        uint32_t index;       /* this block belongs to which block */
        uint32_t begin;       /* offset of the block within piece  */
        char* piece;         /* pointer to start of the piece     */
    };

    void swap(PieceParcel& a, PieceParcel& b);
    bool operator==(PieceParcel const& a, PieceParcel const& b);
}

#endif // !defined(PIECE_PARCEL_HPP)