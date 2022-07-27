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

    std::string t;
    raw_string_ostream rso(t);
    val->getType()->print(rso);

    std::cout << "ID TYPE: " << rso.str() << "\n";

    return nullptr;
}