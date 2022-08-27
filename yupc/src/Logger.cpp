#include "Logger.h"
#include "Compiler/Compiler.h"

#include <cstddef>
#include <fstream>
#include <iostream>

yupc::Logger yupc::GlobalLogger;

void yupc::Logger::LogCompilerError(size_t line, size_t pos, std::string message, 
                                    std::string text, std::string file)
{
    std::cout
        << file << ":" << line << ":" << "pos"
        << "\n"
        << "[" << "COMPILER ERROR" << "]: "
        << message
        << "\n"
        << text
        << "\n";
}

void yupc::Logger::LogCompilerInputError(std::string message)
{
    std::cout
        << "[" << "INPUT ERROR" << "]: "
        << message
        << "\n";
}

void yupc::Logger::LogCompilerInfo(std::string message)
{
    if (yupc::GlobalCompilerOptions.VerboseOutput)
    {
        std::cout
            << "[" << "INFO" << "]: "
            << message
            << "\n";
    }
}

void yupc::Logger::LogLexingError(std::string message, std::string file)
{
    std::cout
        << "["
        << "LEXING ERROR"
        << "]: "
        << " (in file "
        << file
        << ") "
        << message
        << "\n";
}

void yupc::Logger::LogParsingError(size_t line, size_t pos, std::string message, std::string file)
{
    std::cout
        << file << ":" << line << ":" << pos
        <<  "\n"
        << "[" << "PARSING ERROR" << "]: "
        << " (in file "
        << file
        << ") "
        << message
        << "\n";
}
