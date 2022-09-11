#ifndef YUPC_COMPILER_ADDRESS_OF_H_
#define YUPC_COMPILER_ADDRESS_OF_H_

#include "llvm/IR/Value.h"

#include <string>

namespace yupc 
{
    llvm::Value *AddressOfCodegen(std::string id);

} // namespace yupc

#endif
