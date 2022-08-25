#include "compiler/Compiler.h"
#include "compiler/Visitor.h"
#include "compiler/Import.h"
#include "compiler/CompilationUnit.h"
#include "compiler/Type.h"
#include "tree/TerminalNode.h"

#include "llvm/ADT/StringRef.h"
#include <llvm/Support/Casting.h>
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Type.h"


#include <cstddef>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

void yupc::ImportFunctions(llvm::Module &currentModule, llvm::Module &prevModule) 
{
    for (llvm::Function &func : currentModule.functions()) {

        if (func.isPrivateLinkage(func.getLinkage())) {
            continue;
        }

        std::string returnTypeString;
        llvm::raw_string_ostream returnTypeRSO(returnTypeString);

        func.getReturnType()->print(returnTypeRSO);
        llvm::Type *returnType = yupc::ResolveType(returnTypeString, prevModule.getContext());

        std::vector<llvm::Type*> paramTypes;
        for (size_t i = 0; i < func.arg_size(); i++) 
        {
            std::string typeName;
            llvm::raw_string_ostream paramTypeRSO(typeName);
            
            func.getFunctionType()->getParamType(i)->print(paramTypeRSO);

            llvm::Type *paramType = yupc::ResolveType(typeName, prevModule.getContext());

            paramTypes.push_back(paramType);
        }

        llvm::FunctionType *functionType = llvm::FunctionType::get(returnType, paramTypes, func.isVarArg());
        llvm::Function::LinkageTypes linkageType = llvm::Function::ExternalLinkage;
        llvm::FunctionCallee functionCallee = prevModule.getOrInsertFunction(func.getName(), functionType);
        llvm::Function *createdFunction = llvm::cast<llvm::Function*>(functionCallee);

        yupc::CompilationUnits[yupc::CompilationUnits.size() - 2]->Functions[func.getName().str()] = createdFunction;
    }
}

void yupc::ImportGlobalVariables(llvm::Module &currentModule, llvm::Module &prevModule) 
{
    for (llvm::GlobalVariable &gvar : currentModule.globals())
    {
        if (gvar.isPrivateLinkage(gvar.getLinkage()))
        {
            continue;
        }

        std::string gvarTypeString;
        llvm::raw_string_ostream rso(gvarTypeString);
        gvar.getValueType()->print(rso);
        auto *gvar_type = yupc::ResolveType(gvarTypeString, prevModule.getContext());

        bool is_const = gvar.isConstant();

        llvm::GlobalValue::LinkageTypes linkageType = llvm::GlobalValue::ExternalLinkage;
        llvm::GlobalVariable *globalVariable = new llvm::GlobalVariable(prevModule, gvar_type, is_const, 
                                                                        linkageType, nullptr, gvar.getName());

        yupc::CompilationUnits[yupc::CompilationUnits.size() - 2]->GlobalVariables[gvar.getName().str()] = globalVariable;
    }
}

void yupc::ImportTypeAliases(std::vector<yupc::AliasType*> &aliasTypes, int i) {

    yupc::AliasType *aliasType = aliasTypes[i];

    yupc::CompilationUnits.back()->TypeAliases.push_back(aliasType);
}

void yupc::ImportPathRecursively(std::string path)
{
    if (fs::is_directory(path))
    {
        for (auto &entry : fs::directory_iterator(path))
        {
            yupc::ImportPathRecursively(entry.path().string());
        }
    }
    else
    {
        yupc::ProcessPath(path);

        yupc::ImportFunctions(*yupc::CompilationUnits.back()->Module, 
                    *yupc::CompilationUnits[yupc::CompilationUnits.size() - 2]->Module);

        yupc::ImportGlobalVariables(*yupc::CompilationUnits.back()->Module, 
                        *yupc::CompilationUnits[yupc::CompilationUnits.size() - 2]->Module);

        yupc::CompilationUnits.pop_back();
    }
}

std::any yupc::Visitor::visitImportDeclaration(yupc::YupParser::ImportDeclarationContext *ctx) {
    
    std::string moduleName = ctx->ValueString()->getText();
    moduleName = moduleName.substr(1, moduleName.size() - 2);

    yupc::ImportPathRecursively(moduleName);

    return nullptr;
}
