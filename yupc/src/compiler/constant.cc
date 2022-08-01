#include "compiler/visitor.h"
#include "messaging/errors.h"

#include "boost/lexical_cast.hpp"

using namespace llvm;
using namespace boost;
using namespace YupCompiler;

void int_codegen(int64_t value)
{
    ConstantInt *constant = value > INT32_MAX || value < INT32_MIN
            ? ConstantInt::get(Type::getInt64Ty(context), value)
            : ConstantInt::get(Type::getInt32Ty(context), value);
    value_stack.push(constant);
}

void float_codegen(float value)
{
    Value *constant = ConstantFP::get(context, APFloat(value));
    value_stack.push(constant);
}

void bool_codegen(bool value)
{
    Value *constant = ConstantInt::get(Type::getInt8Ty(context), value);
    value_stack.push(constant);
}

void char_codegen(std::string text)
{
    char *cstr = new char[text.length() + 1];
    strcpy(cstr, &text.c_str()[1]);

    Value *constant = ConstantInt::get(
        Type::getInt8Ty(context), *cstr);
    value_stack.push(constant);

    delete []cstr;
}

void string_codegen(std::string text)
{
    Constant *gstrptr = ir_builder.CreateGlobalStringPtr(StringRef(text));
    value_stack.push(gstrptr);
}

std::any Visitor::visitConstant(Parser::YupParser::ConstantContext *ctx)
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
        std::string text = ctx->V_STRING()->getText();
        text.erase(0, 1);
        text.erase(text.size() - 1);
;
        string_codegen(text);

        return nullptr;
    }

    log_compiler_err("couldn't match type and create a constant");
    exit(1);
}