#include <messaging/errors.h>
#include <termcolor/termcolor.hpp>
#include <iostream>

void logCompilerError(std::string &detail)
{
    std::cout << termcolor::red << "[COMPILER ERROR]: " << detail << termcolor::reset << "\n";
}