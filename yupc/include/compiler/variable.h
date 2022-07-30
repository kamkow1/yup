#pragma once

#include "string.h"

void identExpr_codegen(std::string id);

void assignment_codegen(std::string name, llvm::Value *val);

void varDeclare_codegen(std::string name, llvm::Type *resolvedType, 
                        bool isConst, llvm::Value *val = nullptr);
