#include "compiler/visitor.h"
#include "string.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "map"
#include "stack"

using namespace llvm;

std::string module_name;

LLVMContext context;
IRBuilder<> ir_builder(context);
std::unique_ptr<Module> module = std::make_unique<Module>(module_name, context);
std::stack<std::map<std::string, llvm::AllocaInst*>> symbol_table;

std::stack<Value*> value_stack;

// visitor entry point
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

std::any Visitor::visitEmphExpr(YupParser::EmphExprContext *ctx)
{
    return this->visit(ctx->expr());
}
