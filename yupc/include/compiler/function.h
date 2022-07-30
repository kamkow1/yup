#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "vector"
#include "string.h"

void funcDef_codegen(llvm::Function *function);

void funcCall_codegen(std::string funcName, 
                    size_t exprLength, std::vector<Value*> args);

void funcSig_codegen(std::string name, Type *returnType, 
    std::vector<Type*> paramTypes, std::vector<FuncParam*> params);
