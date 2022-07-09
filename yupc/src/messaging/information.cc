#include <messaging/information.h>
#include <termcolor/termcolor.hpp>
#include <iostream>

void logCommandInformation(std::string &detail)
{
    std::cout
        << termcolor::green
        << "["
        << termcolor::underline
        << "INFO"
        << termcolor::reset
        << termcolor::green
        << "]: "
        << termcolor::reset
        << detail
        << termcolor::reset
        << "\n";
}