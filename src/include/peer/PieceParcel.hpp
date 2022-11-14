#if !defined(PIECE_PARCEL_HPP)
#define PIECE_PARCEL_HPP

namespace BT
{
    /* A message used to transfer data */
    /* The transfer of a piece of file happens block by block */
    struct PieceParcel
    {
        long index;       /* this block belongs to which block */
        long begin;       /* offset of the block within piece  */
        char* piece;      /* pointer to start of the piece     */

        PieceParcel();
        PieceParcel(long const i, long const b, char const * const p);

        PieceParcel(PieceParcel const& other);
        PieceParcel(PieceParcel&& other);
        PieceParcel& operator=(PieceParcel other);

        ~PieceParcel();

    private:
        void reset();
    };

    void swap(PieceParcel& a, PieceParcel& b);
    bool operator==(PieceParcel const& a, PieceParcel const& b);
}

#endif // !defined(PIECE_PARCEL_HPP)