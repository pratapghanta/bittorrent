#if !defined(REQUEST_PARCEL_HPP)
#define REQUEST_PARCEL_HPP

namespace BT
{
    struct RequestParcel
    {
        RequestParcel();
        RequestParcel(long const i, long const b, long const l);

        unsigned int Size() const { return sizeof(index) + sizeof(begin) + sizeof(length); }

        long index;       /* which piece */
        long begin;       /* offset within piece */
        long length;      /* number of bytes in the piece */
    };

    bool operator==(RequestParcel const& a, RequestParcel const& b);
}

#endif // !defined(REQUEST_PARCEL_HPP)
