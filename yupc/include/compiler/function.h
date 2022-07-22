#pragma once

#include "llvm/IR/Type.h"
#include "string.h"

struct FuncParam
{
    llvm::Type *paramType;
    std::string paramName;
};

