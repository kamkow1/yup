#pragma once

#include "string"

namespace yupc::msg::errors
{
    void log_compiler_err(std::string detail);

    void log_input_err(std::string detail);

    void log_parsing_err(std::string detail);

    void log_cmd_err(std::string detail);
}