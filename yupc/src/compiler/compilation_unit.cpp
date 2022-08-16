#include <compiler/compilation_unit.h>
#include <compiler/type.h>
#include <compiler/import.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#include <stack>
#include <string>
#include <vector>

using namespace yupc;
using namespace llvm;

namespace com_un = compiler::compilation_unit;
namespace ct = compiler::type;
namespace ci = compiler::import;

std::vector<com_un::CompilationUnit*> com_un::comp_units;

void com_un::init_comp_unit(CompilationUnit &comp_unit) {
    comp_unit.module_id = "";
    comp_unit.module_name = "";
    comp_unit.context = new LLVMContext;
    comp_unit.ir_builder = new IRBuilder<>(*comp_unit.context);
    comp_unit.module = new Module(comp_unit.module_name, *comp_unit.context);
    comp_unit.symbol_table = std::vector<std::map<std::string, AllocaInst*>>{};
    comp_unit.global_variables = std::map<std::string, GlobalVariable*>{};
    comp_unit.functions = std::map<std::string, Function*>{};
    comp_unit.value_stack = std::stack<Value*>{};
    comp_unit.imported_syms = std::vector<ci::ImportDecl>{};
    comp_unit.alias_types = std::vector<ct::AliasType*>{};
}
