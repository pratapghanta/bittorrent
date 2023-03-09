#include "peer/RequestParcel.hpp"

namespace BT 
{
    RequestParcel::RequestParcel() 
        : index(0), 
          begin(0), 
          length(0) 
    {}
        
    RequestParcel::RequestParcel(uint32_t const i, uint32_t const b, uint32_t const l) 
        : index(i), 
          begin(b), 
          length(l) 
    {}

    bool operator==(RequestParcel const& a, RequestParcel const& b) 
    {
        return (a.index == b.index) && 
               (a.begin == b.begin) && 
               (a.length == b.length);
    }
}
