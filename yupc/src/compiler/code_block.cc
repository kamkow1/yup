#include "compiler/visitor.h"

using namespace YupCompiler;

std::any Visitor::visitCode_block(Parser::YupParser::Code_blockContext *ctx)
{
    for (Parser::YupParser::StatementContext *const statement : ctx->statement())
    {
        this->visit(statement);
    }

    return nullptr;
}