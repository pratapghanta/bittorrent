#if !defined(DEFAULTS_HPP)
#define DEFAULTS_HPP

#include <array>
#include <iostream>
#include <string>

namespace BT 
{
	namespace Defaults 
	{
		unsigned int constexpr MaxConnections = 5;
		unsigned int constexpr MaxBufferSize = 1024;
		unsigned int constexpr InitPort = 6677;
		unsigned int constexpr MaxPort = 6699;
		unsigned int constexpr DefaultPort = 6767;
		unsigned int constexpr BlockSize = 16*1024;
		unsigned int constexpr Sha1MdSize = 20;
		std::string const TorrentFileExtension = ".torrent";
		std::string const DefaultLogFilename = "bt_client.log";
		
		int constexpr BadFD = -1;
		std::string const HandshakeMessage = std::string({ 19 }) + "BitTorrent Protocol00000000";

		std::string const HelpMessage = std::string(
											"bittorrent [OPTIONS] file.torrent\n"
											"  -h            \t Print the help screen\n"
											"  -b port       \t Bind to this port for incoming connections\n"
											"  -s save_file  \t Save the torrent in directory save_dir (dflt: .)\n"
											"  -l log_file   \t Save logs to log_file (dflt: bt_client.log)\n"
											"  -p ip:port    \t Instead of contacting the tracker for a peer list,\n"
											"                \t use this peer instead, ip:port (ip or hostname)\n"
											"                \t (include multiple -p for more than 1 peer)\n"
											"  -I id         \t Set the node identifier to id (dflt: random)\n"
											"  -v            \t verbose, print additional verbose info\n");
	}

	void printDefaults(std::ostream&);

	using CharBuffer = std::array<char, Defaults::MaxBufferSize>;
}

#endif // !defined(DEFAULTS_HPP)

