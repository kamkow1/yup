#include "visitor.h"

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