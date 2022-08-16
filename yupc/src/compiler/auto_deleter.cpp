#include <compiler/auto_deleter.h>
#include <compiler/compilation_unit.h>

#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>

#include <vector>

using namespace llvm;
using namespace yupc;

namespace auto_del = compiler::auto_deleter;
namespace com_un = compiler::compilation_unit;

std::vector<AllocaInst*> auto_del::marked_vars;

void auto_del::mark_variable(std::string name) {

    auto *variable = com_un::comp_units.back()->symbol_table.back()[name];

    auto_del::marked_vars.push_back(variable);
}

void auto_del::free_call_codegen(int i) {

    Instruction *free_call = CallInst::CreateFree(auto_del::marked_vars[i], com_un::comp_units.back()->ir_builder->GetInsertBlock());
}
