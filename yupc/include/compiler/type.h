#pragma once

#include "string.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

struct TypeAnnotation
{
    std::string type_str;
    llvm::Value *array_size;
};

llvm::Type *resolve_type(std::string type_name);

std::string get_readable_type_name(std::string type_name);

void check_value_type(llvm::Value *val, std::string name);
