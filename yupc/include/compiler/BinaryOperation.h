#ifndef YUPC_COMPILER_BINARY_OPERATION_H_
#define YUPC_COMPILER_BINARY_OPERATION_H_

#include "llvm/IR/Value.h"

namespace yupc
{
    void BinaryOparationCodegen(llvm::Value *lhs, llvm::Value *rhs, std::string op);

} // namespace yupc

#endif
