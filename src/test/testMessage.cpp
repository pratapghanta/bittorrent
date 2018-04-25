#include <gtest/gtest.h>

#include "Message.hpp"

TEST(testMessageAttributes, chokedMsg) {
	EXPECT_TRUE(BT::Message_t::getChokedMessage().isChoked());
}

TEST(testMessageAttributes, unChokedMsg) {
	EXPECT_TRUE(BT::Message_t::getUnChokedMessage().isUnChoked());
}

TEST(testMessageAttributes, interestedMsg) {
	EXPECT_TRUE(BT::Message_t::getInterestedMessage().isInterested());
}

TEST(testMessageAttributes, notInterestedMsg) {
	EXPECT_TRUE(BT::Message_t::getNotInterestedMessage().isNotInterested());
}

TEST(testMessageAttributes, haveMsg) {
	auto const msg = BT::Message_t::getHaveMessage(10);
	EXPECT_TRUE(msg.isHave());
	EXPECT_EQ(msg.getHave(), 10);
}

TEST(testMessageAttributes, bitfieldMsg) {
	std::string bitfield = "This is a bitfield message";
	auto const msg = BT::Message_t::getBitfieldMessage(bitfield);
	EXPECT_TRUE(msg.isBitfield());
	EXPECT_EQ(msg.getBitfield(), bitfield);
	EXPECT_EQ(msg.getLength(), bitfield.length());
}

TEST(testMessageAttributes, pieceMsg) {
	std::string pieceText = "This is piece text";
	auto pieceValue = BT::Piece_t(10, 1024, pieceText.c_str());
	auto const msg = BT::Message_t::getPieceMessage(pieceValue);
	
	EXPECT_TRUE(msg.isPiece());
	EXPECT_TRUE(msg.getPiece() == pieceValue);
}

TEST(testMessageAttributes, pieceMsg_rValue) {
	std::string pieceText = "This is piece text";
	auto const msg = BT::Message_t::getPieceMessage(BT::Piece_t(10, 1024, pieceText.c_str()));
	EXPECT_TRUE(msg.isPiece());
	EXPECT_TRUE(msg.getPiece() == BT::Piece_t(10, 1024, pieceText.c_str()));
}

TEST(testMessageAttributes, requestMsg) {
	BT::Request_t request(10, 0, 1024);
	auto const msg = BT::Message_t::getRequestMessage(request);
	EXPECT_TRUE(msg.isRequest());
	EXPECT_EQ(msg.getLength(), 1 + sizeof(request.index) + sizeof(request.begin) + sizeof(request.length));
	EXPECT_EQ(msg.getRequest(), request);
}

TEST(testMessageAttributes, keepaliveMessageLength) {
	EXPECT_TRUE(BT::Message_t::getKeepAliveMessage().getLength() == 0);
}
