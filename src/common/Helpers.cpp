#include <iostream>
#include <sstream>
#include <string>
#include <openssl/sha.h>

#include "common/Defaults.hpp"
#include "common/Helpers.hpp"

void ThrowErrorAndExit(std::string const& errorMsg) {
	std::cerr << "ERROR: " << errorMsg << std::endl;
	exit(-1);
}

std::string const CalculateId(std::string const& ip, unsigned int const& port) {
	std::ostringstream oss;
	oss << ip << port;

	std::string const ipAndPort = oss.str();
	
	char hash[BT::Defaults::Sha1MdSize] = "";
	SHA1(reinterpret_cast<unsigned char const *>(ipAndPort.c_str()), ipAndPort.length(), reinterpret_cast<unsigned char *>(hash));
	hash[BT::Defaults::Sha1MdSize-1] = '\0';

    return std::string(hash);
}