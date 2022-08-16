#include <compiler/visitor.h>
#include <compiler/addr_of.h>
#include <compiler/compilation_unit.h>

#include <boost/algorithm/string.hpp>

using namespace llvm;
using namespace boost;
using namespace yupc;

namespace cv = compiler::visitor;
namespace cao = compiler::addr_of;
namespace com_un = compiler::compilation_unit;

void cao::addr_of_expr_codegen(std::string id) {
    auto *val = com_un::comp_units.back()->symbol_table.back()[id];

    com_un::comp_units.back()->value_stack.push(val);
}

std::any cv::Visitor::visitAddr_of(parser::YupParser::Addr_ofContext *ctx) {

    auto text = ctx->IDENTIFIER()->getText();
    cao::addr_of_expr_codegen(text);
    
    return nullptr;
}
