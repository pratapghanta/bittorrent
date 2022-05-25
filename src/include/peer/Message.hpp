#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>

/* The client or server receive different types of messages. This module  */
/* represents the type of messages that the client and server exchange.   */

namespace BT {
	struct Request_t
	{
		long index;       /* which piece */
		long begin;       /* offset within piece */
		long length;      /* number of bytes in the piece */

		Request_t() : index(0), begin(0), length(0) {}
		Request_t(long const i, long const b, long const l) : index(i), begin(b), length(l) {}
	};

	bool operator==(Request_t const& a, Request_t const& b);

	/* A message used to transfer data */
	/* The transfer of a piece of file happens block by block */
	struct Piece_t
	{
		long index;       /* this block belongs to which block */
		long begin;       /* offset of the block within piece  */
		char* piece;      /* pointer to start of the piece     */

		Piece_t();
		Piece_t(long const i, long const b, char const * const p);

		Piece_t(Piece_t const& other);
		Piece_t(Piece_t&& other);
		Piece_t& operator=(Piece_t other);

		~Piece_t();

	private:
		void reset();
	};

	void swap(Piece_t& a, Piece_t& b);
	bool operator==(Piece_t const& a, Piece_t const& b);

	/* This datastructure organizes the data that is transferred between the peers.   */
	/* The acutal trasnfer will only use the required data.                           */
	class Message_t
	{
	public:
		enum class MessageType : char {
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

	private:
		MessageType type;           /* type of bit torrent mesage               */
		unsigned long length;       /* length of the remaining message          */
									/* 0 length message is a keep-alive message */
		union data
		{
			std::string bitfield;   /* a message sent by a peer to tell         */
									/* which piece it has                       */
			long have;              /* an update sent after obtaining a piece   */
			Piece_t   piece;        /* Container to hold piece data             */
			Request_t request;      /* a message sent to request piece          */
			Request_t cancel;       /* cancel the transfer                      */

			data();
			~data();
		} payload;

	private:
		void reset();

	public:
		Message_t() : type(MessageType::END), length(0) {}
		Message_t(Message_t const& other);
		Message_t(Message_t&& other);
		Message_t& operator=(Message_t const&);
		Message_t& operator=(Message_t&&);
		~Message_t();

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
		Piece_t const getPiece() const;
		Request_t const getRequest() const;
		Request_t const getCancel() const;

		static Message_t const getChokedMessage();
		static Message_t const getUnChokedMessage();
		static Message_t const getInterestedMessage();
		static Message_t const getNotInterestedMessage();
		static Message_t const getKeepAliveMessage();
		static Message_t const getBitfieldMessage(std::string const& bitfield);
		static Message_t const getPieceMessage(Piece_t const& piece);
		static Message_t const getHaveMessage(long const have);
		static Message_t const getRequestMessage(Request_t const& request);
		static Message_t const getCancelMessage(Request_t const& cancel);
	};
}

#endif
