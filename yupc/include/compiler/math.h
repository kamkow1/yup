#pragma once

#include <llvm/IR/Value.h>

namespace yupc::compiler::math
{
    void math_oper_expr_codegen(llvm::Value *lhs, 
        llvm::Value *rhs, std::string op);
}
