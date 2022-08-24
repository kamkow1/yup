#pragma once

#include <llvm/IR/Constant.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DataLayout.h>

#include <vector>

namespace yupc 
{
    void indexed_access_expr_codegen(llvm::Value *array, llvm::Value *idxVal);

    void arr_elem_assignment_codegen(std::string arrName, size_t idxNestingLvl, 
                                std::vector<llvm::Value*> idxVals);

    void array_codegen(std::vector<llvm::Constant*> elems, size_t elemCount);
}
