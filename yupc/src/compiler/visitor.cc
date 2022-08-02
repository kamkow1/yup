#include "compiler/visitor.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/DerivedTypes.h"

#include "string.h"
#include "map"
#include "stack"

using namespace llvm;
using namespace yupc;

namespace cv = compiler::visitor;

std::string cv::module_name;

LLVMContext cv::context;
IRBuilder<> cv::ir_builder(context);
std::unique_ptr<Module> cv::module = std::make_unique<Module>(module_name, context);

std::stack<std::map<std::string, llvm::AllocaInst*>> cv::symbol_table;

std::map<std::string, llvm::GlobalVariable*> cv::global_variables;

std::stack<Value*> cv::value_stack;

// visitor entry point
std::any compiler::visitor::Visitor::visitFile(parser::YupParser::FileContext *ctx)
{
    std::vector<parser::YupParser::StatementContext*> statements = ctx->statement();
    for (int i = 0; i < statements.size(); ++i)
    {
        parser::YupParser::StatementContext* statement = statements[i];
        this->visit(statement);
    }

    return ctx;
}

std::any compiler::visitor::Visitor::visitEmphExpr(parser::YupParser::EmphExprContext *ctx)
{
    return this->visit(ctx->expr());
}
