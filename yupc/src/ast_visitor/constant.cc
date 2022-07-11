#include <visitor.h>
#include <messaging/errors.h>

std::any Visitor::visitConstant(YupParser::ConstantContext *ctx)
{
    if (ctx->V_INT() != nullptr)
    {
        std::string text = ctx->V_INT()->getText();
        int value = std::atoi(text.c_str());
        llvm::ConstantInt* constant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(codegenCtx), value);
        valueStack.push(constant);
        return nullptr;
    }

    if (ctx->V_FLOAT() != nullptr)
    {
        std::string text = ctx->V_FLOAT()->getText();
        float value = std::atof(text.c_str());
        llvm::Value* constant = llvm::ConstantFP::get(codegenCtx, llvm::APFloat(value));
        valueStack.push(constant);
        return nullptr;
    }

    if (ctx->V_BOOL() != nullptr)
    {
        std::string text = ctx->V_BOOL()->getText();
        bool value = text == "True";
        llvm::Value* constant = llvm::ConstantInt::get(llvm::Type::getInt8Ty(codegenCtx), value);
        valueStack.push(constant);
        return nullptr;
    }

    std::string errorMessage = "couldn't match type and create a constant";
    logCompilerError(errorMessage);
    exit(1);
}