#include "visitor.h"
#include "boost/algorithm/string.hpp"

using namespace llvm;
using namespace boost;

std::any Visitor::visitAddr_of(YupParser::Addr_ofContext *ctx)
{

    std::string text = ctx->expr()->getText();
    algorithm::erase_all(text, "&");

    AllocaInst *val = symbolTable.top()[text];

    valueStack.push(val);
    return nullptr;
}