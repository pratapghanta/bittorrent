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

#include "common/Defaults.hpp"
#include "peer/BinaryFileHandler.hpp"
#include "peer/MessageParcel.hpp"
#include "peer/MessageParcelFactory.hpp"
#include "peer/Seeder.hpp"

namespace {
	std::string const getDataFilename(std::string const& torrentFilename) {
		unsigned int endPos = torrentFilename.rfind(BT::Defaults::TorrentFileExtension);
		return torrentFilename.substr(0, endPos);
	}
}

BT::Seeder_t::LeecherHandler_t::LeecherHandler_t(BT::Torrent const& t, BT::Peer_t& seeder, BT::Peer_t& leecher)
	: torrent(t), seeder(std::move(seeder)), leecher(std::move(leecher)) {}

bool const BT::Seeder_t::LeecherHandler_t::communicatePortocolMessages(void) {
	bool const handshakeFailed = false;

	leecher.Send(BT::Defaults::HandshakeMessage.c_str(), BT::Defaults::HandshakeMessage.length());
	leecher.Send(torrent.GetInfoHash().c_str(), BT::Defaults::Sha1MdSize - 1);
	leecher.Send(seeder.GetId().c_str(), BT::Defaults::Sha1MdSize - 1);

	char buffer[BT::Defaults::MaxBufferSize] = "";
	memset(buffer, 0, BT::Defaults::MaxBufferSize);
	leecher.Receive(buffer, BT::Defaults::HandshakeMessage.length());

	if (std::string(buffer).compare(BT::Defaults::HandshakeMessage) != 0)
		return handshakeFailed;

	auto inSameSwarm = [&]() {
		memset(buffer, 0, BT::Defaults::MaxBufferSize);
		leecher.Receive(buffer, BT::Defaults::Sha1MdSize - 1);
		return torrent.GetInfoHash().compare(buffer) == 0;
	};

	auto isExpectedHost = [&]() {
		memset(buffer, 0, BT::Defaults::MaxBufferSize);
		leecher.Receive(buffer, BT::Defaults::Sha1MdSize - 1);
		return leecher.GetId().compare(buffer) == 0;
	};

	return inSameSwarm() && isExpectedHost();
}

void BT::Seeder_t::LeecherHandler_t::StartTransfer(void) {
	if (!communicatePortocolMessages())
		return;

	long totalBytesTransferred = 0;
	MessageParcelFactory mpf;

	std::string const avaliablePieces(torrent.GetNumOfPieces(), '1');
	leecher.SendMessage(mpf.GetBitfieldMessage(avaliablePieces));

	auto msg = leecher.ReceiveMessage(MessageType::INTERESTED);
	while (msg.IsInterested()) {
		leecher.SendMessage(mpf.GetUnChokedMessage());

		auto requestMsg = leecher.ReceiveMessage(MessageType::REQUEST);
		BT::RequestParcel const request = requestMsg.GetRequest();

		BT::CBinaryFileHandler fileHndl(getDataFilename(torrent.GetFileName()));
		fileHndl.Seek((request.index * request.length) + request.begin);

		long block = 0;
		long bytesTransfered = 0;
		while (bytesTransfered < request.length)
		{
			if (bytesTransfered % (BT::Defaults::BlockSize) == 0)
			{
				block++;

				BT::PieceParcel piece(request.index, bytesTransfered + 1, nullptr);
				BT::MessageParcel const& pieceMsg = mpf.GetPieceMessage(piece);
				leecher.SendMessage(pieceMsg);

				auto keepAlive = leecher.ReceiveMessage(MessageType::INTERESTED);
			}

			std::string const& data = fileHndl.Get();
			if (data.empty())
				break;
			leecher.Send(data.c_str(), data.length());
			bytesTransfered += data.length();
		}

		totalBytesTransferred += bytesTransfered;

		msg = leecher.ReceiveMessage(MessageType::INTERESTED);
		if (msg.IsKeepAlive()) usleep(1000000);
	}
}

