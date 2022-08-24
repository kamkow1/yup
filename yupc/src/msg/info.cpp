#include <msg/info.h>
#include <iostream>

void yupc::log_cmd_info(std::string detail)
{
    std::cout
        << "["
        << "INFO"
        << "]: "
        << detail
        << "\n";
}