#include "visitor.h"

std::any Visitor::visitCode_block(YupParser::Code_blockContext *ctx)
{
    for (YupParser::StatementContext *const statement : ctx->statement())
    {
        this->visit(statement);
    }

    return nullptr;
}