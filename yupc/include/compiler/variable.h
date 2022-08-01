#pragma once

#include "string.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"

void ident_expr_codegen(std::string id);

void assignment_codegen(std::string name, llvm::Value *val);

void var_declare_codegen(std::string name, llvm::Type *resolved_type, 
                        bool is_const, llvm::Value *val = nullptr);
