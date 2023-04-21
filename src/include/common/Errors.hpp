#if !defined(EXCEPTION_HPP)
#define EXCEPTION_HPP

#include <exception>
#include <string>

namespace BT
{
    struct CException : public std::exception 
    {
        CException(std::string const& descStr,
                    std::string const& helpStr) 
            : errorMsg("Exception: " + descStr + "\n" +
                       "     Help: " + helpStr)
        {}

        CException(std::string const& descStr) 
            : CException(descStr, "-")
        {}

        const char * what() const throw () 
        {
            return errorMsg.c_str();
        }

        private:
            std::string errorMsg;
    };

    extern void FatalError(std::string const& errorMsg);
}

#endif // !defined(EXCEPTION_HPP)
