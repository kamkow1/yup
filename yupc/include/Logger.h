#ifndef YUPC_LOGGER_H_
#define YUPC_LOGGER_H_

#include <cstddef>
#include <sstream>
#include <string>

namespace yupc
{
    struct Logger
    {
    public:
        std::stringstream *LogStream;
        
        void LogCompilerInfo(std::string message);
        void LogCompilerError(size_t line, size_t pos, std::string message, 
                            std::string text, std::string file);
        void LogCompilerInputError(std::string message);
        void LogParsingError(size_t line, size_t pos, std::string message, std::string file);
        void LogLexingError(std::string message, std::string file);
        void LogCommandExecutionError(std::string message);
    };

    extern Logger GlobalLogger;
}

#endif
