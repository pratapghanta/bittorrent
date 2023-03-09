#if !defined(MESSAGE_PARCEL_HPP)
#define MESSAGE_PARCEL_HPP

#include <string>
#include <variant>
#include <cstdint>

#include "peer/PieceParcel.hpp"
#include "peer/RequestParcel.hpp"

namespace BT {
	/* https://wiki.theory.org/BitTorrentSpecification#Messages */

	using MessageLength = unsigned int;
	using Have = uint32_t;

	/* Payload structure:                                                */
	/* bitfield - a message sent by peer to inform which piece it has    */
	/* have - an update sent after obtaining a piece                     */
	/* piece - container to hold piece data                              */
	/* request/cancel - message sent to request piece or cancel transfer */
	using MessagePayload = std::variant<std::monostate,
					                    std::string,
					                    Have,						
					                    PieceParcel,				
					                    RequestParcel>;

	/* The seeder or leecher communicate using the following types of messages.  */
	enum class MessageType : uint8_t 
	{
		CHOKE = 0,
		UNCHOKE,
		INTERESTED,
		NOTINTERESTED,
		HAVE,
		BITFIELD,
		REQUEST,
		PIECE,
		CANCEL,

		END
	};

	class MessageParcel
	{
	public:
		MessageParcel();
		MessageParcel(MessageType, MessageLength const, MessagePayload const&);
		MessageParcel(MessageParcel const&) = default;
		MessageParcel& operator=(MessageParcel const&) = default;
		MessageParcel(MessageParcel&&) = default;
		MessageParcel& operator=(MessageParcel&&) = default;
		~MessageParcel() = default;

		bool IsChoked() const { return type == MessageType::CHOKE; }
		bool IsUnChoked() const { return type == MessageType::UNCHOKE; }
		bool IsInterested() const { return type == MessageType::INTERESTED; }
		bool IsNotInterested() const { return type == MessageType::NOTINTERESTED; }
		bool IsHave() const { return type == MessageType::HAVE; }
		bool IsBitfield() const { return type == MessageType::BITFIELD; }
		bool IsRequest() const { return type == MessageType::REQUEST; }
		bool IsPiece() const { return type == MessageType::PIECE; }
		bool IsCancel() const { return type == MessageType::CANCEL; }
		bool IsKeepAlive() const { return length == 0; }

		MessageType const GetType() const { return type; }
		MessageLength const GetLength() const { return length; }
		std::string const GetBitfield() const;
		Have const GetHave() const;
		PieceParcel const GetPiece() const;
		RequestParcel const GetRequest() const;
		RequestParcel const GetCancel() const;

	private:
		MessageLength length;       /* length of the remaining message          */
									/* 0 length message is a keep-alive message */
		MessageType type;           /* type of bit torrent mesage               */
		MessagePayload payload;

		void reset();
	};
}

#endif // !defined(MESSAGE_PARCEL_HPP)
