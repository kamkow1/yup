#include <Compiler/Visitor.h>

#include <parser/YupParser.h>

// visitor entry point
std::any yupc::Visitor::visitFile(yupc::YupParser::FileContext *ctx)
{
    for (yupc::YupParser::StatementContext *statement : ctx->statement()) 
    {
        this->visit(statement);
    }

    return ctx;
}

std::any yupc::Visitor::visitEmphasizedExpression(yupc::YupParser::EmphasizedExpressionContext *ctx)
{
    return this->visit(ctx->expression());
}
