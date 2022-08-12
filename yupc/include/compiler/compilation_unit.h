#pragma once

#include <compiler/import.h>

#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/GlobalVariable.h>

#include <string>
#include <map>
#include <stack>
#include <vector>

namespace yupc::compiler::compilation_unit {
    struct CompilationUnit {
        std::string module_id;
        std::string module_name;
        llvm::LLVMContext *context;
        llvm::IRBuilder<> *ir_builder;
        llvm::Module *module;
        std::vector<std::map<std::string, llvm::AllocaInst*>> symbol_table;
        std::map<std::string, llvm::GlobalVariable*> global_variables;
        std::map<std::string, llvm::Function*> functions;
        std::stack<llvm::Value*> value_stack;
        std::vector<yupc::compiler::import::ImportDecl> imported_syms;
    };

    extern std::vector<CompilationUnit*> comp_units;
}
