#ifndef YUPC_COMPILER_CONSTANT_H_
#define YUPC_COMPILER_CONSTANT_H_

#include "llvm/IR/Constant.h"

#include <string>
#include <cstdint>

namespace yupc
{
    llvm::Constant *IntegerCodegen(int64_t value);
    llvm::Constant *FloatCodegen(float value);
    llvm::Constant *BoolCodegen(bool value);
    llvm::Constant *CharCodegen(std::string text);
    llvm::Constant *StringCodegen(std::string text);
    llvm::Constant *NullCodegen();
} // namespace yupc

#endif
