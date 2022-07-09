#include <string>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <map>

std::string moduleName;

llvm::LLVMContext codegenCtx;
llvm::IRBuilder<> irBuilder(codegenCtx);
std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>(moduleName, codegenCtx);
std::map<std::string, llvm::Value*> symbolTable;