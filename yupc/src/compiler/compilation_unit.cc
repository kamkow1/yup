#include <compiler/compilation_unit.h>

#include <llvm/IR/LLVMContext.h>

#include <stack>
#include <string>

using namespace yupc;
using namespace llvm;

namespace com_un = compiler::compilation_unit;

std::stack<com_un::CompilationUnit*> com_un::comp_units;
