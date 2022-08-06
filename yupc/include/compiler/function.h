#pragma once

#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <vector>
#include <string.h>

namespace yupc::compiler::function {
    struct FuncParam
    {
        llvm::Type *param_type;
        std::string param_name;
    };

    void func_def_codegen(llvm::Function *function);

    void func_call_codegen(std::string func_name, size_t expr_length, 
                        std::vector<llvm::Value*> args);

    void func_sig_codegen(bool is_external, std::string name, llvm::Type *return_type, 
        std::vector<llvm::Type*> param_types, std::vector<FuncParam*> params);
}
