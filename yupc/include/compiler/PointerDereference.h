#ifndef YUPC_COMPILER_POINTER_DEREFERENCE_H_
#define YUPC_COMPILER_POINTER_DEREFERENCE_H_

#include "llvm/IR/Value.h"

#include <string>

namespace yupc 
{
    void PointerDereferenceCodegen(llvm::Value *value, std::string text);

} // namespace yup

#endif
