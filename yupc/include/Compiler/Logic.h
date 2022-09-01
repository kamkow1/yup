#ifndef YUPC_COMPILER_LOGIC_H_
#define YUPC_COMPILER_LOGIC_H_

#include "llvm/IR/Value.h"

namespace yupc
{
    llvm::Value *EqualCodegen(llvm::Value *lhs, llvm::Value *rhs, bool isSigned);
    llvm::Value *NotEqualCodegen(llvm::Value *lhs, llvm::Value *rhs, bool isSigned);
    llvm::Value *MoreThanCodegen(llvm::Value *lhs, llvm::Value *rhs, bool isSigned);
    llvm::Value *LessThanCodegen(llvm::Value *lhs, llvm::Value *rhs, bool isSigned);

} // namespace yupc

#endif
