#if !defined(MESSAGE_PARCEL_FACTORY_HPP)
#define MESSAGE_PARCEL_FACTORY_HPP

#include "peer/MessageParcel.hpp"

namespace BT
{
    class MessageParcelFactory
    {
    public:
        MessageParcel const GetChokedMessage() const;
        MessageParcel const GetUnChokedMessage() const;
        MessageParcel const GetInterestedMessage() const;
        MessageParcel const GetNotInterestedMessage() const;
        MessageParcel const GetKeepAliveMessage() const;
        MessageParcel const GetBitfieldMessage(std::string const&) const;
        MessageParcel const GetPieceMessage(PieceParcel const&) const;
        MessageParcel const GetHaveMessage(long const) const;
        MessageParcel const GetRequestMessage(RequestParcel const&) const;
        MessageParcel const GetCancelMessage(RequestParcel const&) const;
    };
}

#endif // !defined(MESSAGE_PARCEL_FACTORY_HPP)
