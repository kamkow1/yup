#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>

#include <string>
#include <map>
#include <stack>

using namespace llvm;
using namespace yupc;

// visitor entry point
std::any compiler::visitor::Visitor::visitFile(parser::YupParser::FileContext *ctx)
{
    for (auto &statement : ctx->statement()) {
        this->visit(statement);
    }

    return ctx;
}

std::any compiler::visitor::Visitor::visitEmphExpr(parser::YupParser::EmphExprContext *ctx)
{
    return this->visit(ctx->expr());
}
