#pragma once

#include <string>
#include <llvm/IR/Type.h>

class TypeAnnotation
{
public:
    std::string typeName;
    uint64_t arrayLen;

    TypeAnnotation(std::string tn, uint64_t al);
};

llvm::Type* matchBasicType(std::string typeName);