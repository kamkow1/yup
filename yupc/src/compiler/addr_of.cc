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
    AllocaInst *val = com_un::comp_units[com_un::current_comp_unit_id]
        ->symbol_table.top()[id];

    com_un::comp_units[com_un::current_comp_unit_id]
        ->value_stack.push(val);
}

std::any cv::Visitor::visitAddr_of(parser::YupParser::Addr_ofContext *ctx) {

    std::string text = ctx->IDENTIFIER()->getText();
    cao::addr_of_expr_codegen(text);
    
    return nullptr;
}
