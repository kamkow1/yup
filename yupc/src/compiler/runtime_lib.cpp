#include <compiler/compilation_unit.h>
#include <compiler/runtime_lib.h>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#define PRINTF_NAME     "printf"
#define GC_MALLOC_NAME  "GC_malloc"
#define MALLOC_NAME     "malloc"
#define FREE_NAME       "free"

void yupc::init_printf(llvm::Module &mod) 
{

    llvm::IntegerType *i32t = llvm::IntegerType::getInt32Ty(mod.getContext());
    llvm::PointerType *i8ptr_t = llvm::Type::getInt8Ty(mod.getContext())->getPointerTo();

    llvm::FunctionType *printf_ft = llvm::FunctionType::get(i32t, i8ptr_t, true);

    mod.getOrInsertFunction(PRINTF_NAME, printf_ft);
}

void yupc::init_gc_malloc(llvm::Module &mod) 
{

    llvm::IntegerType *i8_t = llvm::Type::getInt8Ty(mod.getContext());
    llvm::PointerType *void_ptr_t = llvm::PointerType::get(i8_t, 0);
    llvm::IntegerType *i64_t = llvm::Type::getInt64Ty(mod.getContext());

    auto  *malloc_ft = llvm::FunctionType::get(void_ptr_t, i64_t, false);

    mod.getOrInsertFunction(GC_MALLOC_NAME, malloc_ft);
}

void yupc::init_malloc(llvm::Module &mod) 
{
    llvm::IntegerType *i8_t = llvm::Type::getInt8Ty(mod.getContext());
    llvm::IntegerType *i64_t = llvm::Type::getInt64Ty(mod.getContext());
    llvm::PointerType *void_ptr_t = llvm::PointerType::get(i8_t, 0);

    llvm::FunctionType  *gc_malloc_ft = llvm::FunctionType::get(void_ptr_t, i64_t, false);

    mod.getOrInsertFunction(MALLOC_NAME, gc_malloc_ft);
}

void yupc::init_free(llvm::Module &mod) {

    llvm::IntegerType *i8_t = llvm::Type::getInt8Ty(mod.getContext());
    llvm::PointerType *void_ptr_t = llvm::PointerType::get(i8_t, 0);

    llvm::Type *void_t = llvm::Type::getVoidTy(mod.getContext());

    llvm::FunctionType *free_ft = llvm::FunctionType::get(void_t, void_ptr_t, false);

    mod.getOrInsertFunction(FREE_NAME, free_ft);
}

void yupc::init_runtime_lib(llvm::Module &mod) {

    yupc::init_printf(mod);
    yupc::init_malloc(mod);
    yupc::init_gc_malloc(mod);
    yupc::init_free(mod);
}
