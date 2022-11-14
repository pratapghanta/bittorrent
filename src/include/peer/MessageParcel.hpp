#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <variant>

#include "peer/PieceParcel.hpp"
#include "peer/RequestParcel.hpp"

namespace BT {
	
	/* Payload structure:                                                */
	/* bitfield - a message sent by peer to inform which piece it has    */
	/* have - an update sent after obtaining a piece                     */
	/* piece - container to hold piece data                              */
	/* request/cancel - message sent to request piece or cancel transfer */
	using MessagePayload = std::variant<std::monostate,
					                    std::string,
					                    long,						
					                    PieceParcel,				
					                    RequestParcel>;

	/* The seeder or leecher communicate using the following types of messages.  */
	enum class MessageType : short {
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
		MessageParcel() : type{MessageType::END}, length{0} {}
		MessageParcel(MessageParcel const& other) = default;
		MessageParcel& operator=(MessageParcel const&) = default;
		MessageParcel(MessageParcel&& other) = default;
		MessageParcel& operator=(MessageParcel&&) = default;
		~MessageParcel() = default;

		bool isChoked() const { return type == MessageType::CHOKE; }
		bool isUnChoked() const { return type == MessageType::UNCHOKE; }
		bool isInterested() const { return type == MessageType::INTERESTED; }
		bool isNotInterested() const { return type == MessageType::NOTINTERESTED; }
		bool isHave() const { return type == MessageType::HAVE; }
		bool isBitfield() const { return type == MessageType::BITFIELD; }
		bool isRequest() const { return type == MessageType::REQUEST; }
		bool isPiece() const { return type == MessageType::PIECE; }
		bool isCancel() const { return type == MessageType::CANCEL; }
		bool isKeepAlive() const { return length == 0; }

		MessageType const getType() const { return type; }
		unsigned long const getLength() const { return length; }

		std::string const getBitfield() const;
		long const getHave() const;
		PieceParcel const getPiece() const;
		RequestParcel const getRequest() const;
		RequestParcel const getCancel() const;

		static MessageParcel const getChokedMessage();
		static MessageParcel const getUnChokedMessage();
		static MessageParcel const getInterestedMessage();
		static MessageParcel const getNotInterestedMessage();
		static MessageParcel const getKeepAliveMessage();
		static MessageParcel const getBitfieldMessage(std::string const& bitfield);
		static MessageParcel const getPieceMessage(PieceParcel const& piece);
		static MessageParcel const getHaveMessage(long const have);
		static MessageParcel const getRequestMessage(RequestParcel const& request);
		static MessageParcel const getCancelMessage(RequestParcel const& cancel);

	private:
		MessageType type;           /* type of bit torrent mesage               */
		unsigned long length;       /* length of the remaining message          */
									/* 0 length message is a keep-alive message */
		MessagePayload mPayload;

		void reset();
	};
}

#endif
