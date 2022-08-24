#pragma once

#include <llvm/IR/LLVMContext.h>
#include <string>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <cstddef>
#include <map>
#include <vector>

namespace yupc 
{
    enum BuiltInTypes
    {
        LLVM_I32_TYPE,
        LLVM_I64_TYPE,
        LLVM_I8_TYPE,
        LLVM_FLOAT_TYPE,
        LLVM_VOID_TYPE
    };

    struct AliasType 
    {
        std::string type_name;
        std::string destination;
        bool is_public;
    };

    extern std::map<std::string, BuiltInTypes> builtin_llvm_types;

    llvm::Type *resolve_ptr_type(llvm::Type *base);
    
    llvm::Type *resolve_type(std::string type_name, llvm::LLVMContext &ctx_ref);

    llvm::Type *resolve_fixed_array_type(llvm::Type *base, u_int64_t size);

    llvm::Type *get_builtin_llvm_type(std::string type_name, llvm::LLVMContext &ctx_ref);

    bool check_value_type(llvm::Value *val1, llvm::Value *val2);

    std::string type_to_string(llvm::Type *type);
}
