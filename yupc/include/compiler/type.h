#pragma once

#include <llvm/IR/LLVMContext.h>
#include <string>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <cstddef>
#include <map>

namespace yupc 
{
    enum BuiltInTypes 
    {
        LLVM_I32_TYPE,
        LLVM_I64_TYPE,
        LLVM_FLOAT_TYPE,
        LLVM_VOID_TYPE,
        LLVM_I8_TYPE
    };

    struct AliasType 
    {
        std::string type_name;
        std::string destination;
        bool is_public;
    };

    llvm::Type *resolve_ptr_type(llvm::Type *base);
    
    size_t resolve_basic_type(std::string match);

    void appendTypeID(size_t n, std::string id_str);

    llvm::Type *resolve_type(std::string type_name, llvm::LLVMContext &ctx_ref);

    llvm::Type *resolve_fixed_array_type(llvm::Type *base, u_int64_t size);

    std::string get_readable_type_name(std::string type_name);

    void check_value_type(llvm::Value *val, std::string name);
}
