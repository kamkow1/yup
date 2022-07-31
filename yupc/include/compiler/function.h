#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "vector"
#include "string.h"

struct FuncParam
{
    llvm::Type *paramType;
    std::string paramName;
};

void funcDef_codegen(llvm::Function *function);

void funcCall_codegen(std::string funcName, 
                    size_t exprLength, std::vector<llvm::Value*> args);

void funcSig_codegen(bool isExternal, std::string name, llvm::Type *returnType, 
    std::vector<llvm::Type*> paramTypes, std::vector<FuncParam*> params);
