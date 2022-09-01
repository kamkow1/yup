#ifndef YUPC_COMPILER_BINARY_OPERATION_H_
#define YUPC_COMPILER_BINARY_OPERATION_H_

#include "llvm/IR/Value.h"

namespace yupc
{
    llvm::Value *BinaryOparationCodegen(llvm::Value *lhs, llvm::Value *rhs, 
                                        std::string op, bool isSigned);
    llvm::Value *AddCodegen(llvm::Value *lhs, llvm::Value *rhs, bool isSigned);
    llvm::Value *SubCodegen(llvm::Value *lhs, llvm::Value *rhs, bool isSigned);
    llvm::Value *MulCodegen(llvm::Value *lhs, llvm::Value *rhs, bool isSigned);
    llvm::Value *DivCodgen(llvm::Value  *lhs, llvm::Value *rhs, bool isSigned);

} // namespace yupc

#endif
