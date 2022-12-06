#if !defined(ERRORS_HPP)
#define ERRORS_HPP

#include <exception>
#include <string>

namespace BT
{
    struct CException : std::exception 
    {
        CException(std::string const& descStr,
                    std::string const& helpStr) 
            : errorMsg("Exception: " + descStr + "\n" +
                       "    Help:  " + helpStr)
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
}

#endif // !defined(ERRORS_HPP)
