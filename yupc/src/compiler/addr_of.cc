#include "compiler/visitor.h"
#include "compiler/addr_of.h"
#include "boost/algorithm/string.hpp"

using namespace llvm;
using namespace boost;

void addr_of_expr_codegen(std::string id)
{
    AllocaInst *val = symbol_table.top()[id];

    value_stack.push(val);
}

std::any Visitor::visitAddr_of(YupParser::Addr_ofContext *ctx)
{

    std::string text = ctx->IDENTIFIER()->getText();
    addr_of_expr_codegen(text);
    
    return nullptr;
}