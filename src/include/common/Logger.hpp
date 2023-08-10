#if !defined(LOGGER_HPP)
#define LOGGER_HPP

#include <cstdarg>

namespace BT 
{
    enum class LogType : unsigned char 
    {
#if defined(_DEBUG)
        Trace,
#endif // defined(_DEBUG)
        Info,
        Warn,
        Error
    };

    void Log(LogType const, char const * const, ...);
    
#if defined(_DEBUG)    
    #define Trace(formatStr, ...) Log(LogType::Trace, formatStr, ##__VA_ARGS__)
#else
    #define Trace(formatStr, ...)
#endif // defined(_DEBUG)

    #define Info(formatStr, ...)  Log(LogType::Info,  formatStr, ##__VA_ARGS__)
    #define Warn(formatStr, ...)  Log(LogType::Warn,  formatStr, ##__VA_ARGS__)
    #define Error(formatStr, ...) Log(LogType::Error, formatStr, ##__VA_ARGS__)
}

#endif // !defined(LOGGER_HPP)
