#pragma once

#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DataLayout.h"

#include "vector"

namespace yupc::compiler::array
{
    void indexed_access_expr_codegen(llvm::Value *array, llvm::Value *idxVal);

    void arr_elem_assignment_codegen(std::string arrName, size_t idxNestingLvl, 
                                std::vector<llvm::Value*> idxVals);

    void array_codegen(std::vector<llvm::Value*> elems, size_t elemCount);

    llvm::Instruction *create_array_dyn_malloc(llvm::Type *elem_type, 
                            llvm::Value *elem_count, llvm::DataLayout dl);

    llvm::Instruction *create_array_const_malloc(llvm::Type *elem_type, 
                            size_t elem_count, llvm::DataLayout dl);
}
