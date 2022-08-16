#include <msg/errors.h>
#include <termcolor/termcolor.hpp>
#include <iostream>

using namespace yupc::msg;

void errors::log_compiler_err(std::string detail)
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

void errors::log_input_err(std::string detail)
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

void errors::log_parsing_err(std::string detail)
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

void errors::log_cmd_err(std::string detail)
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