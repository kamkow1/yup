#include <compiler/compilation_unit.h>

#include <string>

using namespace yupc;

namespace com_un = compiler::compilation_unit;

std::stack<com_un::CompilationUnit> com_un::comp_units;
