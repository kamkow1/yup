#ifndef YUPC_COMPILER_POINTER_DEREFERENCE_H_
#define YUPC_COMPILER_POINTER_DEREFERENCE_H_

#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"

#include <cstddef>
#include <string>

namespace yupc 
{
    llvm::LoadInst *PointerDereferenceCodegen(llvm::Value *value, size_t line, 
                                    size_t pos, std::string text);

} // namespace yup

#endif
