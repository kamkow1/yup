#include "compiler/CompilationUnit.h"
#include "compiler/Type.h"
#include "compiler/Import.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

#include <memory>
#include <stack>
#include <string>
#include <vector>

std::vector<yupc::CompilationUnit*> yupc::CompilationUnits;

void yupc::InitializeCompilationUnit(yupc::CompilationUnit *compilationUnit, std::string sourceFile)
{
    compilationUnit->SourceFile         = sourceFile;
    compilationUnit->ModuleName         = "";
    compilationUnit->Context            = new llvm::LLVMContext;
    compilationUnit->IRBuilder          = new llvm::IRBuilder<>(*compilationUnit->Context);
    compilationUnit->Module             = new llvm::Module(compilationUnit->ModuleName, *compilationUnit->Context);
    compilationUnit->SymbolTable        = std::vector<std::map<std::string, llvm::AllocaInst*>>();
    compilationUnit->GlobalVariables    = std::map<std::string, llvm::GlobalVariable*>();
    compilationUnit->Functions          = std::map<std::string, llvm::Function*>();
    compilationUnit->ValueStack         = std::stack<llvm::Value*>();
    compilationUnit->TypeStack          = std::stack<llvm::Type*>();
    compilationUnit->TypeAliases        = std::vector<yupc::AliasType*>();

    /*yupc::CompilationUnit *compilationUnit = new yupc::CompilationUnit
    {
        sourceFile,
        "",
        new llvm::LLVMContext,
        new llvm::IRBuilder<>(*compilationUnit->Context),
        new llvm::Module(compilationUnit->ModuleName, *compilationUnit->Context),
        std::vector<std::map<std::string, llvm::AllocaInst*>>(),
        std::map<std::string, llvm::GlobalVariable*>(),
        std::map<std::string, llvm::Function*>(),
        std::stack<llvm::Value*>(),
        std::stack<llvm::Type*>(),
        std::vector<yupc::AliasType*>()
    };

    return compilationUnit;*/
}
