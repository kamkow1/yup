#include <compiler/compilation_unit.h>
#include <compiler/gen_runtime_lib.h>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#define PRINTF_NAME     "printf"
#define GC_MALLOC_NAME  "GC_malloc"
#define MALLOC_NAME     "malloc"
#define FREE_NAME       "free"

using namespace yupc;
using namespace llvm;

namespace grlib = compiler::gen_runtime_lib;

void grlib::init_printf(Module &mod) {

    auto *i32t = IntegerType::getInt32Ty(mod.getContext());
    auto *i8ptr_t = Type::getInt8Ty(mod.getContext())->getPointerTo();

    auto *printf_ft = FunctionType::get(i32t, i8ptr_t, true);

    mod.getOrInsertFunction(PRINTF_NAME, printf_ft);
}

void grlib::init_gc_malloc(Module &mod) {

    auto *i8_t = Type::getInt8Ty(mod.getContext());
    auto *void_ptr_t = PointerType::get(i8_t, 0);
    auto *i64_t = Type::getInt64Ty(mod.getContext());

    auto  *malloc_ft = FunctionType::get(void_ptr_t, i64_t, false);

    mod.getOrInsertFunction(GC_MALLOC_NAME, malloc_ft);
}

void grlib::init_malloc(Module &mod) {

    auto *i8_t = Type::getInt8Ty(mod.getContext());
    auto *void_ptr_t = PointerType::get(i8_t, 0);
    auto *i64_t = Type::getInt64Ty(mod.getContext());

    auto  *gc_malloc_ft = FunctionType::get(void_ptr_t, i64_t, false);

    mod.getOrInsertFunction(MALLOC_NAME, gc_malloc_ft);
}

void grlib::init_free(Module &mod) {

    auto *i8_t = Type::getInt8Ty(mod.getContext());
    auto *void_ptr_t = PointerType::get(i8_t, 0);

    auto *void_t = Type::getVoidTy(mod.getContext());

    auto *free_ft = FunctionType::get(void_t, void_ptr_t, false);

    mod.getOrInsertFunction(FREE_NAME, free_ft);
}

void grlib::init_runtime_lib(Module &mod) {

    grlib::init_printf(mod);
    grlib::init_malloc(mod);
    grlib::init_free(mod);
}
