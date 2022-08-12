#include "compiler/compilation_unit.h"
#include <compiler/module.h>
#include <compiler/visitor.h>

#include <vector>
#include <string>

using namespace yupc;

namespace cv = compiler::visitor;
namespace cm = compiler::module;
namespace com_un = compiler::compilation_unit;

void cm::set_module_id(std::string new_id) {
    com_un::comp_units.back()->module_id = new_id;
}

std::any cv::Visitor::visitModule_decl(parser::YupParser::Module_declContext *ctx) {

    auto module_name = ctx->IDENTIFIER()->getText();
    
    com_un::comp_units.back()->module_id = module_name;
    return nullptr;
}
