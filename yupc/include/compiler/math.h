#pragma once

#include "llvm/IR/Value.h"

void math_oper_expr_codegen(llvm::Value *lhs, 
    llvm::Value *rhs, std::string op);
