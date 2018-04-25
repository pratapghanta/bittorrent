#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h> //ip hdeader library (must come before ip_icmp.h)
#include <netinet/ip_icmp.h> //icmp header
#include <arpa/inet.h> //internet address library
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <openssl/sha.h>

#include "Peer.hpp"
#include "Message.hpp"
#include "Leecher.hpp"
#include "Defaults.hpp"
#include "helpers.hpp"
#include "BinaryFileHandler.hpp"

namespace {
	std::string const getSaveFilenameForPiece(std::string const& saveFile, long const pieceIndex) {
		std::stringstream ss;
		unsigned int const endPos = saveFile.rfind(BT::Defaults::torrentFileExtension);
		ss << saveFile.substr(0, endPos) << "_" << pieceIndex;

		return ss.str();
	}

	bool const isPieceAvailableAtSeeder(BT::Message_t const& msg, long const pieceIndex) {
		auto piecesInfo = msg.getBitfield();
		return piecesInfo[pieceIndex] == 1;
	}

	bool isTransferSuccessful(BT::Torrent_t const& torrent, long const pieceIndex, std::string const& fileContents) {
		unsigned char hash[BT::Defaults::Sha1MdSize] = "";

		SHA1(reinterpret_cast<unsigned char const*>(fileContents.c_str()), fileContents.length(), hash);
		for (unsigned int i = 0; i < BT::Defaults::Sha1MdSize - 1; i++)
			if (hash[i] == '\0')
				hash[i] = '_';

		return torrent.getPieceHashes()[pieceIndex].compare(reinterpret_cast<char*>(hash));
	}

	BT::Peer_t getClientDetails(int const sockfd) {
		sockaddr_in cli_addr;
		socklen_t len = BT::Defaults::IPSize - 1;

		if (getsockname(sockfd, (sockaddr*)&cli_addr, &len) == -1)
			return BT::Peer_t("", 0);

		char buffer[BT::Defaults::IPSize] = "";
		inet_ntop(cli_addr.sin_family, (void *)&cli_addr.sin_addr, buffer, BT::Defaults::IPSize - 1);
		std::string ip(buffer);

		unsigned int port = ntohs(cli_addr.sin_port);

		return BT::Peer_t(sockfd, ip, port);
	}

	BT::Peer_t establishConnectionToSeeder(std::string const& ip, unsigned int const port) {
		int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sockfd < 0)
			return BT::Peer_t("", 0);

		sockaddr_in serv_addr;
		memset((void *)&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
		serv_addr.sin_port = htons(port);

		if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) != 0)
			return BT::Peer_t("", 0);

		return std::move(getClientDetails(sockfd));
	}
}

BT::Leecher_t::Leecher_t(BT::Torrent_t const& t, Peer_t& s)
	: torrent(t), seeder(std::move(s)), leecher(std::move(establishConnectionToSeeder(s.getIp(), s.getPort()))) {
}

bool const BT::Leecher_t::communicatePortocolMessages() {
	seeder.send(BT::Defaults::handshakeMessage.c_str(), BT::Defaults::handshakeMessage.length());
	seeder.send(torrent.getInfoHash().c_str(), BT::Defaults::Sha1MdSize - 1);
	seeder.send(leecher.getId().c_str(), BT::Defaults::Sha1MdSize - 1);

	char buffer[BT::Defaults::MaxBufferSize] = "";
	seeder.receive(buffer, BT::Defaults::handshakeMessage.length());
	if (BT::Defaults::handshakeMessage.compare(buffer) != 0)
		return false;

	auto inSameSwarm = [&]() {
		memset(buffer, 0, BT::Defaults::MaxBufferSize);
		seeder.receive(buffer, BT::Defaults::Sha1MdSize - 1);
		return torrent.getInfoHash().compare(buffer) == 0;
	};

	auto expectedSeeder = [&]() {
		memset(buffer, 0, BT::Defaults::MaxBufferSize);
		seeder.receive(buffer, BT::Defaults::Sha1MdSize - 1);
		return seeder.getId().compare(buffer) == 0;
	};

	return inSameSwarm() && expectedSeeder();
}

bool const BT::Leecher_t::getPieceFromSeeder(long const interestedPiece) {
	auto msg = seeder.receiveMessage(BT::Message_t::MessageType::BITFIELD);

	if (!isPieceAvailableAtSeeder(msg, interestedPiece)) {
		seeder.sendMessage(Message_t::getNotInterestedMessage());
		return false;
	}

	seeder.sendMessage(Message_t::getInterestedMessage());
	msg = seeder.receiveMessage(BT::Message_t::MessageType::CHOKE); /* Expecting choke/unchoke */
	if (msg.isChoked()) return false;

	int const begin = 0;
	auto requestDetails = BT::Request_t(interestedPiece, begin, torrent.getPieceLength());
	seeder.sendMessage(Message_t::getRequestMessage(requestDetails));

	auto isEndOfPiece = [&](long const currPos) { return currPos >= torrent.getPieceLength(); };
	auto isEndOfDataFile = [&](long const currPos) { return (interestedPiece * torrent.getPieceLength()) + begin + currPos >= torrent.getFileLength(); };

	std::string fileContents;
	while (!isEndOfPiece(fileContents.length()) && !isEndOfDataFile(fileContents.length())) {
		if (fileContents.length() % BT::Defaults::BlockSize == 0) {
			auto pieceMsg = seeder.receiveMessage(BT::Message_t::MessageType::PIECE);
			if (!(pieceMsg.getPiece() == BT::Piece_t(interestedPiece, fileContents.length(), nullptr)))
				return false;
			seeder.sendMessage(BT::Message_t::getKeepAliveMessage());
		}
		char dataBuf[2] = "\0";
		seeder.receive(dataBuf, 1);
		fileContents += std::string(dataBuf);
	}

	BT::BinaryFileHandler_t fileHndl(getSaveFilenameForPiece(torrent.getFileName(), interestedPiece));
	fileHndl.put(fileContents);

	return isTransferSuccessful(torrent, interestedPiece, fileContents);
}

void BT::Leecher_t::startTransfer() {
	if (!communicatePortocolMessages())
		return;

	long interestedPiece = 1;

	bool const isTransferred = getPieceFromSeeder(interestedPiece);
	if (isTransferred) {
		/* Broadcast to all other peers */
		/* Print to log about the downloaded piece */
		/* Synchronize threads such that this piece is not downloaded again */
		seeder.sendMessage(Message_t::getNotInterestedMessage());
	}
}
