#include "../../include/visitor.h"

std::any Visitor::visitFile(YupParser::FileContext *ctx)
{
    //module = llvm::Ma

    std::vector<YupParser::StatementContext*> statements = ctx->statement();
    for (int i = 0; i < statements.size(); ++i)
    {
        YupParser::StatementContext* statement = statements[i];
        this->visit(statement);
    }

    return ctx;
}

std::any Visitor::visitFunc_def(YupParser::Func_defContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();
    std::cout << name << "\n";

    return 0;
}

std::any Visitor::visitConstant(YupParser::ConstantContext *ctx)
{
    if (ctx->V_INT() != nullptr)
    {
        std::string text = ctx->V_INT()->getText();
        int value = std::atoi(text.c_str());
        return llvm::ConstantInt::get(codegenCtx, llvm::APInt(sizeof value, (uint64_t) value, false));
    }

    return logError("couldn't match type and generate proper IR constant");
}

// logger
llvm::Value *logError(const char* str)
{
    std::stringstream message;
    message << str << "\n";

    fprintf(stderr, "%s", message.str().c_str());
    return nullptr;
}