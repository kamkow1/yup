#pragma once

#include <llvm/IR/Type.h>
#include <string>

class FuncParam
{
public:
    llvm::Type* paramType;
    std::string paramName;

    FuncParam(llvm::Type* type, std::string pn);
};

