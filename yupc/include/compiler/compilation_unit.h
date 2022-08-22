#pragma once

#include <compiler/import.h>
#include <compiler/type.h>

#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/GlobalVariable.h>

#include <string>
#include <map>
#include <stack>
#include <vector>

namespace yupc 
{
    struct CompilationUnit 
    {
        std::string source_file;
        std::string module_name;
        llvm::LLVMContext *context;
        llvm::IRBuilder<> *ir_builder;
        llvm::Module *module;
        std::vector<std::map<std::string, llvm::AllocaInst*>> symbol_table;
        std::map<std::string, llvm::GlobalVariable*> global_variables;
        std::map<std::string, llvm::Function*> functions;
        std::stack<llvm::Value*> value_stack;
        std::stack<llvm::Type*> type_stack;
        std::vector<yupc::ImportDecl> imported_syms;
        std::vector<yupc::AliasType*> alias_types;
    };

    extern std::vector<CompilationUnit*> comp_units;

    void init_comp_unit(CompilationUnit &comp_unit, std::string source_file);
}
