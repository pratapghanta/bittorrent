#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdarg>

namespace {
    enum LogType {
        LT_TRACE,
        LT_INFO,
        LT_WARN,
        LT_ERROR
    };

    void log(LogType const type, char const * const pMsgFormat, ...) {
        int constexpr MAX_LOG_MESSAGE_SIZE = 1024;
        std::vector<std::string> const importance {
            "Trace", "Info", "Warn", "Error"
        };
        
        va_list args;
        va_start(args, pMsgFormat);
        static char messageBuffer[MAX_LOG_MESSAGE_SIZE] = "";
        vsprintf(messageBuffer, pMsgFormat, args);
        va_end(args);

        if (LT_ERROR == type) {
            std::cerr << importance[type] << ":\t" << messageBuffer << std::endl;
            return;
        }
        std::cout << importance[type] << messageBuffer << std::endl;
    }
}

namespace BT {
    #define Trace(formatStr, ...) log(LogType::LT_TRACE, formatStr, ##__VA_ARGS__)
    #define Info(formatStr, ...) log(LogType::LT_INFO, formatStr, ##__VA_ARGS__)
    #define Warn(formatStr, ...) log(LogType::LT_WARN, formatStr, ##__VA_ARGS__)
    #define Error(formatStr, ...) log(LogType::LT_ERROR, formatStr, ##__VA_ARGS__)
}

#endif // #ifndef LOGGER_HPP
