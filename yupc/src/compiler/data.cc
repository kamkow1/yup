#include "string.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "map"
#include "stack"

using namespace llvm;

std::string moduleName;

LLVMContext context;
IRBuilder<> irBuilder(context);
std::unique_ptr<Module> module = std::make_unique<Module>(moduleName, context);
std::stack<std::map<std::string, llvm::AllocaInst*>> symbolTable;

std::stack<Value*> valueStack;
