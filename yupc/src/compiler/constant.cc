#include <visitor.h>
#include <messaging/errors.h>
#include <boost/lexical_cast.hpp>

std::any Visitor::visitConstant(YupParser::ConstantContext *ctx)
{
    if (ctx->V_INT() != nullptr)
    {
        std::string text = ctx->V_INT()->getText();
        int64_t value = boost::lexical_cast<uint64_t>(text.c_str());
        
        llvm::ConstantInt* constant = value > INT32_MAX || value < INT32_MIN
            ? llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegenCtx), value)
            : llvm::ConstantInt::get(llvm::Type::getInt32Ty(codegenCtx), value);
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

    if (ctx->V_CHAR() != nullptr)
    {
        std::string text = ctx->V_CHAR()->getText();
        char *cstr = new char[text.length() + 1];
        strcpy(cstr, &text.c_str()[1]);
        llvm::Value* constant = llvm::ConstantInt::get(llvm::Type::getInt8Ty(codegenCtx), *cstr);
        valueStack.push(constant);

        delete []cstr;
        return nullptr;
    }

    /*if (ctx->V_STRING() != nullptr)
    {
        std::string text = ctx->V_STRING()->getText();
        std::cout << text << "\n";
    }*/

    std::string errorMessage = "couldn't match type and create a constant";
    logCompilerError(errorMessage);
    exit(1);
}