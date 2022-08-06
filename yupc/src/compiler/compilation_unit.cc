#include <compiler/compilation_unit.h>

#include <string.h>
#include <map>

using namespace yupc;

namespace com_un = compiler::compilation_unit;

std::map<std::string, com_un::CompilationUnit*> com_un::comp_units;
std::string com_un::current_comp_unit_id;
std::stack<std::string> com_un::comp_unit_id_stack;
