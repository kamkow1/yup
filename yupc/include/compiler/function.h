#pragma once

#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <vector>
#include <string>

namespace yupc 
{
    struct FuncParam
    {
        llvm::Type *param_type;
        std::string param_name;
    };

    void func_def_codegen(llvm::Function *function);

    void func_call_codegen(std::string func_name, std::vector<llvm::Value*> args, std::string text);

    void func_sig_codegen(bool is_external, std::string name, llvm::Type *return_type, 
                        std::vector<llvm::Type*> param_types, std::vector<FuncParam*> params);
}
