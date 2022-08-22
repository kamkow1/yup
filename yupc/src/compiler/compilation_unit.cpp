#include <compiler/compilation_unit.h>
#include <compiler/type.h>
#include <compiler/import.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>

#include <stack>
#include <string>
#include <vector>

std::vector<yupc::CompilationUnit*> yupc::comp_units;

void yupc::init_comp_unit(yupc::CompilationUnit &comp_unit, std::string source_file) 
{
    comp_unit.source_file = source_file;
    comp_unit.module_name = "";
    comp_unit.context = new llvm::LLVMContext;
    comp_unit.ir_builder = new llvm::IRBuilder<>(*comp_unit.context);
    comp_unit.module = new llvm::Module(comp_unit.module_name, *comp_unit.context);
    comp_unit.symbol_table = std::vector<std::map<std::string, llvm::AllocaInst*>>();
    comp_unit.global_variables = std::map<std::string, llvm::GlobalVariable*>();
    comp_unit.functions = std::map<std::string, llvm::Function*>();
    comp_unit.value_stack = std::stack<llvm::Value*>();
    comp_unit.type_stack = std::stack<llvm::Type*>();
    comp_unit.imported_syms = std::vector<yupc::ImportDecl>();
    comp_unit.alias_types = std::vector<yupc::AliasType*>();
}
