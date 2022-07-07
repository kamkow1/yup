#include "visitor.h"
#include <vector>
#include <iostream>

std::any Visitor::visitFile(YupParser::FileContext *ctx)
{
    std::vector<YupParser::StatementContext*> statements = ctx->statement();
    for (int i = 0; i < statements.size(); ++i)
    {
        YupParser::StatementContext* statement = statements[i];
        this->visit(statement);
    }

    return ctx;
}

std::any Visitor::visitFunc_def(YupParser::Func_defContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();
    std::cout << name << "\n";

    return 0;
}