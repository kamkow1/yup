#pragma once

#include "string.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

struct TypeAnnotation
{
    std::string typeName;
};

llvm::Type *resolveType(std::string typeName);

std::string getReadableTypeName(std::string typeName);

void checkValueType(llvm::Value *val, std::string name);
