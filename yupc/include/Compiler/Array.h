#ifndef YUPC_COMPILER_ARRAY_H_
#define YUPC_COMPILER_ARRAY_H_

#include "llvm/IR/Constant.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"

#include <vector>

namespace yupc 
{
    llvm::Value *ArrayPointerIndexedAccessCodegen(llvm::Value *array, llvm::Value *idxVal,
                                                size_t line, size_t pos, std::string text);
    llvm::Value *ConstArrayIndexedAccessCodegen(llvm::Value *array, llvm::Value *idxVal);
    llvm::Constant *ArrayCodegen(std::vector<llvm::Constant*> elems);
    void ArrayElementAssignmentCodegen(std::string arrName, size_t idxNestingLvl, 
                                std::vector<llvm::Value*> idxVals);
} // namespace yupc

#endif
