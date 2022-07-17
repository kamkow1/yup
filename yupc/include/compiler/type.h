#pragma once

#include <string>
#include <llvm/IR/Type.h>

struct TypeAnnotation
{
    std::string typeName;
};

llvm::Type* resolveType(std::string typeName);