#include <visitor.h>

std::any Visitor::visitConstant(YupParser::ConstantContext *ctx)
{
    if (ctx->V_INT() != nullptr)
    {
        std::string text = ctx->V_INT()->getText();
        int value = std::atoi(text.c_str());
        llvm::ConstantInt* constantInt
            = llvm::ConstantInt::get(codegenCtx, llvm::APInt(32,  value, false));
        return constantInt;
    }

    fprintf(stderr, "ERROR: couldn't match type and generate proper IR constant\n");
    exit(1);
}