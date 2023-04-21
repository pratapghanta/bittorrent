#if !defined(HELPERS_HPP)
#define HELPERS_HPP

#include <string>

extern std::string const CalculateId(const std::string& ip, const unsigned int& port);
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

#endif // !defined(HELPERS_HPP)
