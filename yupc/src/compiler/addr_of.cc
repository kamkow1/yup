#include "compiler/visitor.h"
#include "compiler/addr_of.h"

#include "boost/algorithm/string.hpp"

using namespace llvm;
using namespace boost;
using namespace yupc;

namespace cv = compiler::visitor;
namespace cao = compiler::addr_of;

void cao::addr_of_expr_codegen(std::string id)
{
    AllocaInst *val = cv::symbol_table.top()[id];

    cv::value_stack.push(val);
}

std::any cv::Visitor::visitAddr_of(parser::YupParser::Addr_ofContext *ctx)
{

    std::string text = ctx->IDENTIFIER()->getText();
    cao::addr_of_expr_codegen(text);
    
    return nullptr;
}