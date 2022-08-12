#pragma once

#include "string.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

namespace yupc::compiler::type {
    enum BuiltInTypes {
        I32_TYPE,
        I64_TYPE,
        I8_TYPE,
        FLOAT_TYPE,
        BOOL_TYPE,
        VOID_TYPE,
        CHAR_TYPE
    };

    static llvm::Type *resolve_ptr_type(llvm::Type *base);

    static size_t resolve_basic_type(std::string match);

    void appendTypeID(size_t n, std::string id_str);

    llvm::Type *resolve_type(std::string type_name);

    std::string get_readable_type_name(std::string type_name);

    void check_value_type(llvm::Value *val, std::string name);
}
