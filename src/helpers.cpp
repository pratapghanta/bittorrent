#include <iostream>
#include <sstream>
#include <string>
#include <openssl/sha.h>

#include "Defaults.hpp"
#include "helpers.hpp"

std::string const getHelpMesssage() {
	return std::string(
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

void throwErrorAndExit(std::string const& errorMsg) {
	std::cerr << "ERROR: " << errorMsg << std::endl;
	exit(-1);
}

std::string const calculateId(std::string const& ip, unsigned int const& port) {
	std::ostringstream oss;
	oss << ip << port;

	std::string const ipAndPort = oss.str();
	
	char hash[BT::Defaults::Sha1MdSize] = "";
	SHA1(reinterpret_cast<unsigned char const *>(ipAndPort.c_str()), ipAndPort.length(), reinterpret_cast<unsigned char *>(hash));
	hash[BT::Defaults::Sha1MdSize-1] = '\0';

    return std::string(hash);
}