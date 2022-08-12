#pragma once

#include "string.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"

namespace yupc::compiler::variable {
    void ident_expr_codegen(std::string id, bool is_glob);

    void assignment_codegen(std::string name, llvm::Value *val);

    void var_declare_codegen(std::string name, llvm::Type *resolved_type, bool is_const, bool is_glob, bool is_ext, llvm::Value *val = nullptr);

}
