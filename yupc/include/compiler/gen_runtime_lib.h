#pragma once

#include <llvm/IR/Module.h>

namespace yupc::compiler::gen_runtime_lib {

    void init_runtime_lib(llvm::Module &mod);

    void init_gc_malloc(llvm::Module &mod);

    void init_malloc(llvm::Module &mod);

    void init_printf(llvm::Module &mod);

    void init_free(llvm::Module &mod);
}
