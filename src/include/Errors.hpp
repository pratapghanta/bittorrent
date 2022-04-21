#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <exception>

namespace BT
{
    struct BTException : std::exception 
    {
        BTException(std::string const& descStr,
                    std::string const& helpStr) 
            : errorMsg("Exception: " + descStr + "\n" +
                       "    Help:  " + helpStr)
        {}

        BTException(std::string const& descStr) 
            : BTException(descStr, "-")
        {}

        const char * what() const throw () 
        {
            return errorMsg.c_str();
        }

        private:
            std::string errorMsg;
    };

    enum STATUSCODE 
    {
        SC_SUCCESS,
        SC_FAIL_UNKNOWN,
        SC_FAIL_BAD_TORRENT
    };

    inline bool SC_FAILED(STATUSCODE const status) { return status; }
}

#endif // #ifndef ERRORS_HPP