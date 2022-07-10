#include <visitor.h>
#include <messaging/errors.h>

std::any Visitor::visitConstant(YupParser::ConstantContext *ctx)
{
    if (ctx->V_INT() != nullptr)
    {
        std::string text = ctx->V_INT()->getText();
        int value = std::atoi(text.c_str());
        valueStack.push(llvm::ConstantInt::get(llvm::Type::getInt32Ty(codegenCtx), value));
        return nullptr;
    }

    std::string errorMessage = "couldn't match type and create a constant";
    logCompilerError(errorMessage);
    exit(1);
}