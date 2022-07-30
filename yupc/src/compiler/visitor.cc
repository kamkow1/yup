#include "visitor.h"
#include "string.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "map"
#include "stack"

using namespace llvm;

std::string moduleName;

LLVMContext context;
IRBuilder<> irBuilder(context);
std::unique_ptr<Module> module = std::make_unique<Module>(moduleName, context);
std::stack<std::map<std::string, llvm::AllocaInst*>> symbolTable;

std::stack<Value*> valueStack;

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