#include <messaging/information.h>
#include <termcolor/termcolor.hpp>
#include <iostream>

void logCommandInformation(std::string detail)
{
    std::cout
        << termcolor::bright_blue
        << "["
        << termcolor::underline
        << "INFO"
        << termcolor::reset
        << termcolor::bright_blue
        << "]: "
        << termcolor::reset
        << detail
        << termcolor::reset
        << "\n";
}