#include "../../include/visitor.h"

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