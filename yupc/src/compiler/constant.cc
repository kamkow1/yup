#include "visitor.h"
#include "messaging/errors.h"
#include "boost/lexical_cast.hpp"

using namespace llvm;
using namespace boost;

void int_codegen(int64_t value)
{
    ConstantInt *constant = value > INT32_MAX || value < INT32_MIN
            ? ConstantInt::get(Type::getInt64Ty(context), value)
            : ConstantInt::get(Type::getInt32Ty(context), value);
    valueStack.push(constant);
}

void float_codegen(float value)
{
    Value *constant = ConstantFP::get(context, APFloat(value));
    valueStack.push(constant);
}

void bool_codegen(bool value)
{
    Value *constant = ConstantInt::get(Type::getInt8Ty(context), value);
    valueStack.push(constant);
}

void char_codegen(std::string text)
{
    char *cstr = new char[text.length() + 1];
    strcpy(cstr, &text.c_str()[1]);
    Value *constant = ConstantInt::get(Type::getInt8Ty(context), *cstr);
    valueStack.push(constant);

    delete []cstr;
}

std::any Visitor::visitConstant(YupParser::ConstantContext *ctx)
{
    if (ctx->V_INT() != nullptr)
    {
        std::string text = ctx->V_INT()->getText();
        int64_t value = lexical_cast<int64_t>(text.c_str());
        
        int_codegen(value);
        
        return nullptr;
    }

    if (ctx->V_FLOAT() != nullptr)
    {
        std::string text = ctx->V_FLOAT()->getText();
        float value = std::atof(text.c_str());
        
        float_codegen(value);

        return nullptr;
    }

    if (ctx->V_BOOL() != nullptr)
    {
        std::string text = ctx->V_BOOL()->getText();
        bool value = text == "True";

        bool_codegen(value);
        
        return nullptr;
    }

    if (ctx->V_CHAR() != nullptr)
    {
        std::string text = ctx->V_CHAR()->getText();

        char_codegen(text);
        
        return nullptr;
    }

    if (ctx->V_STRING() != nullptr)
    {
        std::string text = ctx->V_CHAR()->getText();
        text.erase(0);
        text.erase(text.size() - 1);
        return nullptr;
    }

    logCompilerError("couldn't match type and create a constant");
    exit(1);
}