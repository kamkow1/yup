#ifndef YUPC_COMPILER_COMPILATION_UNIT_H_
#define YUPC_COMPILER_COMPILATION_UNIT_H_

#include "Compiler/Import.h"
#include "Compiler/Type.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/GlobalVariable.h"

#include <string>
#include <map>
#include <stack>
#include <vector>

namespace yupc 
{
    struct CompilationUnit
    {
        std::string SourceFile;
        std::string ModuleName;
        llvm::LLVMContext *Context;
        llvm::IRBuilder<> *IRBuilder;
        llvm::Module *Module;
        std::vector<std::map<std::string, llvm::AllocaInst*>> SymbolTable;
        std::map<std::string, llvm::GlobalVariable*> GlobalVariables;
        std::map<std::string, llvm::Function*> Functions;
        std::stack<llvm::Value*> ValueStack;
        std::stack<llvm::Type*> TypeStack;
        std::vector<yupc::AliasType*> TypeAliases;
    };

    extern std::vector<CompilationUnit*> CompilationUnits;

    void InitializeCompilationUnit(CompilationUnit *compilationUnit, std::string sourceFile);

} // namespace yupc

#endif
