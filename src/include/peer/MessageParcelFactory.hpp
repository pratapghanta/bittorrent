#if !defined(MESSAGE_PARCEL_FACTORY_HPP)
#define MESSAGE_PARCEL_FACTORY_HPP

#include "peer/MessageParcel.hpp"

namespace BT
{
    class MessageParcelFactory
    {
    public:
        static MessageParcel const GetChokedMessage();
        static MessageParcel const GetUnChokedMessage();
        static MessageParcel const GetInterestedMessage();
        static MessageParcel const GetNotInterestedMessage();
        static MessageParcel const GetKeepAliveMessage();
        static MessageParcel const GetBitfieldMessage(std::string const&);
        static MessageParcel const GetPieceMessage(PieceParcel const&);
        static MessageParcel const GetHaveMessage(Have const);
        static MessageParcel const GetRequestMessage(RequestParcel const&);
        static MessageParcel const GetCancelMessage(RequestParcel const&);
    };
}

#endif // !defined(MESSAGE_PARCEL_FACTORY_HPP)
