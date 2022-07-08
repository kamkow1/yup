#include "visitor.h"

std::any Visitor::visitFunc_def(YupParser::Func_defContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();
    std::cout << name << "\n";

    return 0;
}