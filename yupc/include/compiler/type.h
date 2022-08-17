#pragma once

#include <string>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <map>

namespace yupc::compiler::type {
    enum BuiltInTypes {
        I32_TYPE,
        I64_TYPE,
        FLOAT_TYPE,
        VOID_TYPE,
        BYTE_TYPE
    };

    struct AliasType {
        std::string type_name;
        std::string destination;
        bool is_public;
    };

    llvm::Type *resolve_ptr_type(llvm::Type *base);
    
    size_t resolve_basic_type(std::string match);

    void appendTypeID(size_t n, std::string id_str);

    llvm::Type *resolve_type(std::string type_name);

    llvm::Type *resolve_fixed_array_type(llvm::Type *base, uint64_t size);

    std::string get_readable_type_name(std::string type_name);

    void check_value_type(llvm::Value *val, std::string name);
}
