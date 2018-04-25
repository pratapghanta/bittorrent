#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <thread>
#include <map>
#include <memory>

#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <openssl/sha.h>

#include "Seeder.hpp"
#include "Defaults.hpp"
#include "Message.hpp"
#include "BinaryFileHandler.hpp"

namespace {
	std::string const getDataFilename(std::string const& torrentFilename) {
		unsigned int endPos = torrentFilename.rfind(BT::Defaults::torrentFileExtension);
		return torrentFilename.substr(0, endPos);
	}
}

BT::Seeder_t::LeecherHandler_t::LeecherHandler_t(BT::Torrent_t const& t, BT::Peer_t& seeder, BT::Peer_t& leecher)
	: torrent(t), seeder(std::move(seeder)), leecher(std::move(leecher)) {}

bool const BT::Seeder_t::LeecherHandler_t::communicatePortocolMessages(void) {
	bool const handshakeFailed = false;

	leecher.send(BT::Defaults::handshakeMessage.c_str(), BT::Defaults::handshakeMessage.length());
	leecher.send(torrent.getInfoHash().c_str(), BT::Defaults::Sha1MdSize - 1);
	leecher.send(seeder.getId().c_str(), BT::Defaults::Sha1MdSize - 1);

	char buffer[BT::Defaults::MaxBufferSize] = "";
	memset(buffer, 0, BT::Defaults::MaxBufferSize);
	leecher.receive(buffer, BT::Defaults::handshakeMessage.length());

	if (std::string(buffer).compare(BT::Defaults::handshakeMessage) != 0)
		return handshakeFailed;

	auto inSameSwarm = [&]() {
		memset(buffer, 0, BT::Defaults::MaxBufferSize);
		leecher.receive(buffer, BT::Defaults::Sha1MdSize - 1);
		return torrent.getInfoHash().compare(buffer) == 0;
	};

	auto isExpectedHost = [&]() {
		memset(buffer, 0, BT::Defaults::MaxBufferSize);
		leecher.receive(buffer, BT::Defaults::Sha1MdSize - 1);
		return leecher.getId().compare(buffer) == 0;
	};

	return inSameSwarm() && isExpectedHost();
}

void BT::Seeder_t::LeecherHandler_t::doTransfer(void) {
	if (!communicatePortocolMessages())
		return;

	long totalBytesTransferred = 0;

	std::string const avaliablePieces(torrent.getNumOfPieces(), '1');
	leecher.sendMessage(BT::Message_t::getBitfieldMessage(avaliablePieces));

	auto msg = leecher.receiveMessage(BT::Message_t::MessageType::INTERESTED);
	while (msg.isInterested()) {
		leecher.sendMessage(BT::Message_t::getUnChokedMessage());

		auto requestMsg = leecher.receiveMessage(BT::Message_t::MessageType::REQUEST);
		BT::Request_t const request = requestMsg.getRequest();

		BT::BinaryFileHandler_t fileHndl(getDataFilename(torrent.getFileName()));
		fileHndl.seek((request.index * request.length) + request.begin);

		long block = 0;
		long bytesTransfered = 0;
		while (bytesTransfered < request.length)
		{
			if (bytesTransfered % (BT::Defaults::BlockSize) == 0)
			{
				block++;

				BT::Piece_t piece(request.index, bytesTransfered + 1, nullptr);
				BT::Message_t const& pieceMsg = BT::Message_t::getPieceMessage(piece);
				leecher.sendMessage(pieceMsg);

				auto keepAlive = leecher.receiveMessage(BT::Message_t::MessageType::INTERESTED);
			}

			std::string const& data = fileHndl.get();
			if (data.empty())
				break;
			leecher.send(data.c_str(), data.length());
			bytesTransfered += data.length();
		}

		totalBytesTransferred += bytesTransfered;

		msg = leecher.receiveMessage(BT::Message_t::MessageType::INTERESTED);
		if (msg.isKeepAlive()) usleep(1000000);
	}
}

