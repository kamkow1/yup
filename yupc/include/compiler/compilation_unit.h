#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/GlobalVariable.h>

#include <string.h>
#include <map>
#include <stack>

namespace yupc::compiler::compilation_unit {
    struct CompilationUnit {
        std::string module_name;
        llvm::LLVMContext *context;
        llvm::IRBuilder<> ir_builder;
        std::unique_ptr<llvm::Module> module;
        std::stack<std::map<std::string, llvm::AllocaInst*>> symbol_table;
        std::map<std::string, llvm::GlobalVariable*> global_variables;
        std::stack<llvm::Value*> value_stack;
        std::vector<std::string> module_imports;
    };

    extern std::string current_comp_unit_id;
    extern std::map<std::string, CompilationUnit*> comp_units;
    extern std::stack<std::string> comp_unit_id_stack;
}
