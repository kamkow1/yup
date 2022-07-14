#include <visitor.h>

std::any Visitor::visitCode_block(YupParser::Code_blockContext *ctx)
{
    for (const auto &statement : ctx->statement())
    {
        this->visit(statement);
    }

    return nullptr;
}