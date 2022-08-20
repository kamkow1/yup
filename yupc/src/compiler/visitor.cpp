#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>

#include <parser/YupParser.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>

#include <string>
#include <map>
#include <stack>

// visitor entry point
std::any yupc::Visitor::visitFile(yupc::YupParser::FileContext *ctx)
{
    for (yupc::YupParser::StatementContext *statement : ctx->statement()) 
    {
        this->visit(statement);
    }

    return ctx;
}

std::any yupc::Visitor::visitEmphExpr(yupc::YupParser::EmphExprContext *ctx)
{
    return this->visit(ctx->expr());
}
