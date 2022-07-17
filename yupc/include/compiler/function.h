#pragma once

#include <llvm/IR/Type.h>
#include <string>

struct FuncParam
{
    llvm::Type *paramType;
    std::string paramName;
};

