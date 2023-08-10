#include <cstdio>
#include <iostream>
#include <unordered_map>

#include "common/Logger.hpp"

namespace 
{
    std::unordered_map<BT::LogType, std::string> const importanceStrMap {
#if defined(_DEBUG)        
        { BT::LogType::Trace, "Trace" },
#endif // defined(_DEBUG)        
        { BT::LogType::Info,  "Info"  },
        { BT::LogType::Warn,  "Warn"  },
        { BT::LogType::Error, "Error" }
    };

    inline void printMessage(BT::LogType const type, char const * const messageBuffer) 
    {
        if (BT::LogType::Error == type) 
        {
            std::cerr << importanceStrMap.at(type) << ":\t" << messageBuffer << std::endl;
            return;
        }
        std::cout << importanceStrMap.at(type) << ":\t" << messageBuffer << std::endl;
    }
}

namespace BT {
    void Log(LogType const type, char const * const pMsgFormat, ...) 
    {
        static size_t constexpr maxLogMessageSize = 1024 + 1;
        static thread_local char messageBuffer[maxLogMessageSize] = "";

        va_list args;
        va_start(args, pMsgFormat);
        vsprintf(messageBuffer, pMsgFormat, args);
        va_end(args);
        messageBuffer[maxLogMessageSize-1] = '\0';

        printMessage(type, messageBuffer);        
    }
}