#include <msg/errors.h>
#include <termcolor/termcolor.hpp>
#include <iostream>

using namespace yupc;

void msg::errors::log_compiler_err(std::string detail, std::string text)
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
        << "\n"
        << termcolor::underline
        << text
        << termcolor::reset
        << "\n";
}

void msg::errors::log_input_err(std::string detail)
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

void msg::errors::log_parsing_err(std::string detail, std::string f_path)
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
        << " (in file "
        << f_path
        << ") "
        << detail
        << termcolor::reset
        << "\n";
}

void msg::errors::log_lexing_err(std::string detail, std::string f_path) {
    std::cout
        << termcolor::cyan
        << "["
        << termcolor::underline
        << "LEXING ERROR"
        << termcolor::reset
        << termcolor::yellow
        << "]: "
        << termcolor::reset
        << " (in file "
        << f_path
        << ") "
        << detail
        << termcolor::reset
        << "\n";
}

void msg::errors::log_cmd_err(std::string detail)
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