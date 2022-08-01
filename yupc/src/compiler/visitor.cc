#include "compiler/visitor.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"

#include "string.h"
#include "map"
#include "stack"

using namespace llvm;
using namespace YupCompiler;

std::string module_name;

LLVMContext context;
IRBuilder<> ir_builder(context);
std::unique_ptr<Module> module = std::make_unique<Module>(module_name, context);
std::stack<std::map<std::string, llvm::AllocaInst*>> symbol_table;

std::stack<Value*> value_stack;

// visitor entry point
std::any Visitor::visitFile(Parser::YupParser::FileContext *ctx)
{
    std::vector<Parser::YupParser::StatementContext*> statements = ctx->statement();
    for (int i = 0; i < statements.size(); ++i)
    {
        Parser::YupParser::StatementContext* statement = statements[i];
        this->visit(statement);
    }

    return ctx;
}

std::any Visitor::visitEmphExpr(Parser::YupParser::EmphExprContext *ctx)
{
    return this->visit(ctx->expr());
}
