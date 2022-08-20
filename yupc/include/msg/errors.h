#pragma once

#include <string>

namespace yupc 
{
    void log_compiler_err(std::string detail, std::string text);

    void log_input_err(std::string detail);

    void log_parsing_err(std::string detail, std::string f_path);

    void log_lexing_err(std::string detail, std::string f_path);

    void log_cmd_err(std::string detail);
}
