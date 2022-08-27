#include "Compiler/Visitor.h"
#include "Compiler/CompilationUnit.h"
#include "Compiler/Constant.h"
#include "Compiler/Type.h"
#include "Logger.h"
#include "utils.h"

#include "llvm/IR/Constant.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"

#include <string>
#include <cstdint>


llvm::Constant *yupc::IntegerCodegen(int64_t value) 
{
    llvm::IntegerType *i64type = llvm::Type::getInt64Ty(*yupc::CompilationUnits.back()->Context);
    llvm::IntegerType *i32type = llvm::Type::getInt32Ty(*yupc::CompilationUnits.back()->Context);

    llvm::ConstantInt *constant = value > INT32_MAX || value < INT32_MIN
            ? llvm::ConstantInt::get(i64type, value)
            : llvm::ConstantInt::get(i32type, value);

    return constant;
}

llvm::Constant *yupc::FloatCodegen(float value) 
{
    llvm::ConstantFP *constant = llvm::ConstantFP::get(*yupc::CompilationUnits.back()->Context, llvm::APFloat(value));
    return constant;
}

llvm::Constant *yupc::BoolCodegen(bool value) 
{
    llvm::IntegerType *type = llvm::Type::getInt8Ty(*yupc::CompilationUnits.back()->Context);
    llvm::Constant *constant = llvm::ConstantInt::get(type, value);
    return constant;
}

llvm::Constant *yupc::CharCodegen(std::string text) 
{

    char *cstr = new char[text.length() + 1];
    strcpy(cstr, &text.c_str()[1]);

    llvm::IntegerType *type = llvm::Type::getInt8Ty(*yupc::CompilationUnits.back()->Context);
    llvm::Constant *constant = llvm::ConstantInt::get(type, *cstr);

    delete []cstr;
    return constant;
}

llvm::Constant *yupc::StringCodegen(std::string text) 
{
    llvm::Constant *gstrptr = yupc::CompilationUnits.back()->IRBuilder->CreateGlobalStringPtr(llvm::StringRef(text));
    return gstrptr;
}

llvm::Constant *yupc::NullCodegen() 
{
    llvm::Type *voidPtrType = llvm::Type::getInt8Ty(*yupc::CompilationUnits.back()->Context)->getPointerTo();
    llvm::Constant *nullp = llvm::ConstantPointerNull::getNullValue(voidPtrType);
    return nullp;
}

std::any yupc::Visitor::visitConstant(yupc::YupParser::ConstantContext *ctx) 
{
    llvm::Constant *result;

    if (ctx->ValueInteger() != nullptr) 
    {
        std::string text = ctx->ValueInteger()->getText();
        int64_t value = yupc::StringToInt64(text);
        result = yupc::IntegerCodegen(value);
    }
    else if (ctx->ValueFloat() != nullptr) 
    {
        std::string text = ctx->ValueFloat()->getText();
        double value = std::atof(text.c_str());
        result = yupc::FloatCodegen(value);
    }
    else if (ctx->ValueBool() != nullptr) 
    {
        std::string text = ctx->ValueBool()->getText();
        bool value = text == "True";
        result = yupc::BoolCodegen(value);
    }
    else if (ctx->ValueChar() != nullptr) 
    {
        std::string text = ctx->ValueChar()->getText();
        result = yupc::CharCodegen(text);
    }
    else if (ctx->ValueString() != nullptr) 
    {
        std::string text = ctx->ValueString()->getText();
        yupc::StringStripQuotes(text);
        result = yupc::StringCodegen(text);
    }
    else 
    {
        result = yupc::NullCodegen();
    }

    yupc::CompilationUnits.back()->ValueStack.push(result);

    return nullptr;
}
