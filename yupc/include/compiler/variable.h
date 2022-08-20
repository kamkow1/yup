#pragma once

#include "string.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"

namespace yupc 
{

    struct Variable 
    {
        std::string name;
        bool is_const;
        bool is_ref;
    };

    void ident_expr_codegen(std::string id, bool is_glob);

    void assignment_codegen(std::string name, llvm::Value *val, std::string text);

    void var_declare_codegen(std::string name, llvm::Type *resolved_type, bool is_const, bool is_glob, 
                            bool is_ext, bool is_ref, llvm::Value *val = nullptr);

}
