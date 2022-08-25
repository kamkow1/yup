#ifndef YUPC_COMPILER_CODE_BLOCK_H_
#define YUPC_COMPILER_CODE_BLOCK_H_

#include "llvm/IR/Instructions.h"

#include <string>
#include <map>

namespace yupc
{    
    void CreateNewScope();
    void DropScope();

} // namespace yupc

#endif
