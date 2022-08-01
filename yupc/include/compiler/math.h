#pragma once

#include "llvm/IR/Value.h"

void mathOperExpr_codegen(llvm::Value *lhs, 
    llvm::Value *rhs, std::string op);
