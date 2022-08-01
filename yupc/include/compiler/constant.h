#pragma once
#include "string.h"
#include "cstdint"

namespace yupc::compiler::constant
{
    void int_codegen(int64_t value);

    void float_codegen(float value);

    void bool_codegen(bool value);

    void char_codegen(std::string text);

    void string_codegen(std::string text);
}
