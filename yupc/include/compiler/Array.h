#ifndef YUPC_COMPILER_ARRAY_H_
#define YUPC_COMPILER_ARRAY_H_

#include "llvm/IR/Constant.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DataLayout.h"

#include <vector>

namespace yupc 
{
    void ConstArrayIndexedAccessCodegen(llvm::Value *array, llvm::Value *idxVal);

    void ArrayElementAssignmentCodegen(std::string arrName, size_t idxNestingLvl, 
                                std::vector<llvm::Value*> idxVals);

    void ArrayCodegen(std::vector<llvm::Constant*> elems);
} // namespace yupc

#endif
