#pragma once

#include <llvm/IR/Module.h>

namespace yupc 
{

    void init_runtime_lib(llvm::Module &mod);

    void init_gc_malloc(llvm::Module &mod);

    void init_malloc(llvm::Module &mod);

    void init_printf(llvm::Module &mod);

    void init_free(llvm::Module &mod);
}
