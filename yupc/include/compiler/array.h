#pragma once

#include "llvm/IR/Value.h"
#include "vector"

void indexedAccessExpr_codegen(llvm::Value *array, llvm::Value *idxVal);

void arrElemAssignment_codegen(std::string arrName, size_t idxNestingLvl, 
                                std::vector<llvm::Value*> idxVals);

void array_codegen(std::vector<llvm::Value*> elems, size_t elemCount);
