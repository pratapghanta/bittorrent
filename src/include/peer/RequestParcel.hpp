#if !defined(REQUEST_PARCEL_HPP)
#define REQUEST_PARCEL_HPP

#include <cstdint>

namespace BT
{
    struct RequestParcel
    {
        RequestParcel();
        RequestParcel(uint32_t const i, uint32_t const b, uint32_t const l);

        unsigned int Size() const { return sizeof(index) + sizeof(begin) + sizeof(length); }

        uint32_t index;       /* which piece */
        uint32_t begin;       /* offset within piece */
        uint32_t length;      /* number of bytes in the piece */
    };

    bool operator==(RequestParcel const& a, RequestParcel const& b);
}

#endif // !defined(REQUEST_PARCEL_HPP)
