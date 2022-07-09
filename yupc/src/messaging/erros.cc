#include <messaging/errors.h>
#include <termcolor/termcolor.hpp>
#include <iostream>

void logCompilerError(std::string &detail)
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

void logInputError(std::string &detail)
{
    std::cout << termcolor::magenta << "[INPUT ERROR]: " << detail << termcolor::reset << "\n";
}