#include <msg/errors.h>
#include <iostream>

void yupc::log_compiler_err(std::string detail, std::string text)
{
    std::cout
        << "["
        << "COMPILER ERROR"
        <<"]: "
        << detail
        << "\n"
        << text
        << "\n";
}

void yupc::log_input_err(std::string detail)
{
    std::cout
        << "["
        << "INPUT ERROR"
        <<"]: "
        << detail
        << "\n";
}

void yupc::log_parsing_err(std::string detail, std::string f_path)
{
    std::cout
        << "["
        << "PARSING ERROR"
        << "]: "
        << " (in file "
        << f_path
        << ") "
        << detail
        << "\n";
}

void yupc::log_lexing_err(std::string detail, std::string f_path) {
    std::cout
        << "["
        << "LEXING ERROR"
        << "]: "
        << " (in file "
        << f_path
        << ") "
        << detail
        << "\n";
}

void yupc::log_cmd_err(std::string detail)
{
    std::cout
    << "["
    << "CMD ERROR"
    << "]: "
    << detail
    << "\n";
    
}