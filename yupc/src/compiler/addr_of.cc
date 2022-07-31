#include "compiler/visitor.h"
#include "compiler/addr_of.h"
#include "boost/algorithm/string.hpp"

using namespace llvm;
using namespace boost;

void addrOfExpr_codegen(std::string id)
{
    AllocaInst *val = symbolTable.top()[id];

    valueStack.push(val);
}

std::any Visitor::visitAddr_of(YupParser::Addr_ofContext *ctx)
{

    std::string text = ctx->IDENTIFIER()->getText();
    addrOfExpr_codegen(text);
    
    return nullptr;
}