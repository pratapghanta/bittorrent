#if !defined(HELPERS_HPP)
#define HELPERS_HPP

#include <string>

extern void PrintErrorAndExit(std::string const& errorMsg);
extern std::string const CalculateId(const std::string& ip, const unsigned int& port);
extern std::string const GetHelpMesssage();

#endif // !defined(HELPERS_HPP)
