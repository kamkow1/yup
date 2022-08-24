#pragma once

#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>

#include <string>
#include <cstddef>
#include <map>
#include <vector>

namespace yupc 
{

    struct Variable 
    {
        std::string name;
        bool is_const;
        bool is_ref;
    };

    llvm::AllocaInst *find_local_variable(std::string name, size_t i,
        std::vector<std::map<std::string, llvm::AllocaInst*>> &symbol_table, std::string text);

    void ident_expr_codegen(std::string id, bool is_glob);

    void assignment_codegen(std::string name, llvm::Value *val, std::string text);

    void var_declare_codegen(std::string name, llvm::Type *resolved_type, bool is_const, bool is_glob, 
                            bool is_ext, bool is_ref, llvm::Value *val = nullptr);

}
