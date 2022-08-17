#pragma once

#include <llvm/IR/Value.h>
#include <string>

namespace yupc::compiler::ptr_dereference {
    void ptr_deref_codegen(llvm::Value *value, std::string text);
}
