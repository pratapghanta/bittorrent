#if !defined(REQUEST_PARCEL_HPP)
#define REQUEST_PARCEL_HPP

namespace BT
{
    struct RequestParcel
    {
        long index;       /* which piece */
        long begin;       /* offset within piece */
        long length;      /* number of bytes in the piece */

        RequestParcel();
        RequestParcel(long const i, long const b, long const l);
    };

    bool operator==(RequestParcel const& a, RequestParcel const& b);
}

#endif // !defined(REQUEST_PARCEL_HPP)
