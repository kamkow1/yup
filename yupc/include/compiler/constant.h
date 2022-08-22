#pragma once

#include <string>
#include <cstdint>

namespace yupc
{
    void int_codegen(int64_t value);

    void float_codegen(float value);

    void bool_codegen(bool value);

    void char_codegen(std::string text);

    void string_codegen(std::string text);

    void null_codegen();
}
