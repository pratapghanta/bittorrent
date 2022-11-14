#include <gtest/gtest.h>

#include "MessageParcel.hpp"

TEST(testMessageAttributes, chokedMsg) {
	EXPECT_TRUE(BT::MessageParcel::getChokedMessage().isChoked());
}

TEST(testMessageAttributes, unChokedMsg) {
	EXPECT_TRUE(BT::MessageParcel::getUnChokedMessage().isUnChoked());
}

TEST(testMessageAttributes, interestedMsg) {
	EXPECT_TRUE(BT::MessageParcel::getInterestedMessage().isInterested());
}

TEST(testMessageAttributes, notInterestedMsg) {
	EXPECT_TRUE(BT::MessageParcel::getNotInterestedMessage().isNotInterested());
}

TEST(testMessageAttributes, haveMsg) {
	auto const msg = BT::MessageParcel::getHaveMessage(10);
	EXPECT_TRUE(msg.isHave());
	EXPECT_EQ(msg.getHave(), 10);
}

TEST(testMessageAttributes, bitfieldMsg) {
	#if 0
	std::string bitfield = "This is a bitfield message";
	auto const msg = BT::MessageParcel::getBitfieldMessage(bitfield);
	EXPECT_TRUE(msg.isBitfield());
	EXPECT_EQ(msg.getBitfield(), bitfield);
	EXPECT_EQ(msg.getLength(), bitfield.length());
	#endif
}

TEST(testMessageAttributes, pieceMsg) {
	std::string pieceText = "This is piece text";
	auto pieceValue = BT::PieceParcel(10, 1024, pieceText.c_str());
	auto const msg = BT::MessageParcel::getPieceMessage(pieceValue);
	
	EXPECT_TRUE(msg.isPiece());
	EXPECT_TRUE(msg.getPiece() == pieceValue);
}

TEST(testMessageAttributes, pieceMsg_rValue) {
	std::string pieceText = "This is piece text";
	auto const msg = BT::MessageParcel::getPieceMessage(BT::PieceParcel(10, 1024, pieceText.c_str()));
	EXPECT_TRUE(msg.isPiece());
	EXPECT_TRUE(msg.getPiece() == BT::PieceParcel(10, 1024, pieceText.c_str()));
}

TEST(testMessageAttributes, requestMsg) {
	BT::RequestParcel request(10, 0, 1024);
	auto const msg = BT::MessageParcel::getRequestMessage(request);
	EXPECT_TRUE(msg.isRequest());
	EXPECT_EQ(msg.getLength(), 1 + sizeof(request.index) + sizeof(request.begin) + sizeof(request.length));
	EXPECT_EQ(msg.getRequest(), request);
}

TEST(testMessageAttributes, keepaliveMessageLength) {
	EXPECT_TRUE(BT::MessageParcel::getKeepAliveMessage().getLength() == 0);
}
