#ifndef YUPC_COMPILER_BINARY_OPERATION_H_
#define YUPC_COMPILER_BINARY_OPERATION_H_

#include "llvm/IR/Value.h"

namespace yupc
{
    llvm::Value *BinaryOparationCodegen(llvm::Value *lhs, llvm::Value *rhs, std::string op);
    llvm::Value *AddCodegen(llvm::Value *lhs, llvm::Value *rhs);
    llvm::Value *SubCodegen(llvm::Value *lhs, llvm::Value *rhs);
    llvm::Value *MulCodegen(llvm::Value *lhs, llvm::Value *rhs);
    llvm::Value *DivCodgen(llvm::Value  *lhs, llvm::Value *rhs);

} // namespace yupc

#endif
