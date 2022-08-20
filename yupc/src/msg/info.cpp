#include <msg/info.h>
#include <termcolor/termcolor.hpp>
#include <iostream>

void yupc::log_cmd_info(std::string detail)
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