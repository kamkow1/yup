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
    llvm::LoadInst *ConstArrayIndexedAccessCodegen(llvm::Value *array, llvm::Value *idxVal);
    llvm::Constant *ArrayCodegen(std::vector<llvm::Constant*> elems);
    void ArrayElementAssignmentCodegen(std::string arrName, size_t idxNestingLvl, 
                                std::vector<llvm::Value*> idxVals);
} // namespace yupc

#endif
