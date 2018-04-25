#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <string>

extern std::string const calculateId(const std::string& ip, const unsigned int& port);
extern void throwErrorAndExit(std::string const& errorMsg);
extern std::string const getHelpMesssage();

#endif
