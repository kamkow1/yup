#include <messaging/errors.h>
#include <termcolor/termcolor.hpp>
#include <iostream>

void logCompilerError(std::string detail)
{
    std::cout
        << termcolor::red
        << "["
        << termcolor::underline
        << "COMPILER ERROR"
        << termcolor::reset
        << termcolor::red
        <<"]: "
        << termcolor::reset
        << detail
        << termcolor::reset
        << "\n";
}

void logInputError(std::string detail)
{
    std::cout
        << termcolor::magenta
        << "["
        << termcolor::underline
        << "INPUT ERROR"
        << termcolor::reset
        << termcolor::magenta
        <<"]: "
        << termcolor::reset
        << detail
        << termcolor::reset
        << "\n";
}

void logParsingError(std::string detail)
{
    std::cout
        << termcolor::yellow
        << "["
        << termcolor::underline
        << "PARSING ERROR"
        << termcolor::reset
        << termcolor::yellow
        << "]: "
        << termcolor::reset
        << detail
        << termcolor::reset
        << "\n";
}

void logCommandError(std::string detail)
{
    std::cout
    << termcolor::yellow
    << "["
    << termcolor::underline
    << "CMD ERROR"
    << termcolor::reset
    << termcolor::yellow
    << "]: "
    << termcolor::reset
    << detail
    << termcolor::reset
    << "\n";
    
}