#ifndef DEFAULTS_HPP
#define DEFAULTS_HPP

#include <string>

namespace BT {
	namespace Defaults {
		unsigned int constexpr MaxConnections = 5;
		unsigned int constexpr MaxFilenameSize = 1024 + 1;
		unsigned int constexpr MaxBufferSize = 1024 + 1;
		unsigned int constexpr InitPort = 6677;
		unsigned int constexpr MaxPort = 6699;
		unsigned int constexpr DefaultPort = 6767;
		unsigned int constexpr BlockSize = 16 * 1024;
		unsigned int constexpr IPSize = 15 + 1;
		unsigned int constexpr PortSize = 5 + 1;
		unsigned int constexpr Sha1MdSize = 20 + 1;
		std::string const torrentFileExtension = ".torrent";
		std::string const defaultLogFilename = "bt_client.log";
		
		int constexpr BadFD = -1;
		std::string const handshakeMessage = std::string({ 19 }) + "BitTorrent Protocol00000000";
	}
}
#endif

