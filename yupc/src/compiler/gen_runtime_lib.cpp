#include <compiler/compilation_unit.h>
#include <compiler/gen_runtime_lib.h>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

using namespace yupc;
using namespace llvm;

namespace grlib = compiler::gen_runtime_lib;

void grlib::init_printf(Module &mod) {

    auto *i32t = IntegerType::getInt32Ty(mod.getContext());
    auto *i8ptr_t = Type::getInt8Ty(mod.getContext())->getPointerTo();

    auto *printf_ft = FunctionType::get(i32t, i8ptr_t, true);

    mod.getOrInsertFunction("printf", printf_ft);
}

void grlib::init_malloc(Module &mod) {

    auto *i8_t = Type::getInt8Ty(mod.getContext());
    auto *void_ptr_t = PointerType::get(i8_t, 0);
    auto *i64_t = Type::getInt64Ty(mod.getContext());

    auto  *malloc_ft = FunctionType::get(void_ptr_t, i64_t, false);

    mod.getOrInsertFunction("malloc", malloc_ft);
}

void grlib::init_runtime_lib(Module &mod) {

    init_printf(mod);
    init_malloc(mod);
}
