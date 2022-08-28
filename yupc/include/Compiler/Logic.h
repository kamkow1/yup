#ifndef YUPC_COMPILER_LOGIC_H_
#define YUPC_COMPILER_LOGIC_H_

#include "llvm/IR/Value.h"

namespace yupc
{
    llvm::Value *EqualCodegen(llvm::Value *lhs, llvm::Value *rhs);
    llvm::Value *NotEqualCodegen(llvm::Value *lhs, llvm::Value *rhs);

} // namespace yupc

#endif
