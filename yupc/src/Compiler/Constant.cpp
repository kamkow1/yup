#include "Compiler/Visitor.h"
#include "Compiler/CompilationUnit.h"
#include "Compiler/Constant.h"
#include "Compiler/Type.h"
#include "Logger.h"
#include "utils.h"

#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Constants.h>

#include <string>
#include <cstdint>


void yupc::IntegerCodegen(int64_t value) 
{
    llvm::IntegerType *i64type = llvm::Type::getInt64Ty(*yupc::CompilationUnits.back()->Context);
    llvm::IntegerType *i32type = llvm::Type::getInt32Ty(*yupc::CompilationUnits.back()->Context);

    llvm::ConstantInt *constant = value > INT32_MAX || value < INT32_MIN
            ? llvm::ConstantInt::get(i64type, value)
            : llvm::ConstantInt::get(i32type, value);

    yupc::CompilationUnits.back()->ValueStack.push(constant);
}

void yupc::FloatCodegen(float value) 
{
    llvm::ConstantFP *constant = llvm::ConstantFP::get(*yupc::CompilationUnits.back()->Context, llvm::APFloat(value));

    yupc::CompilationUnits.back()->ValueStack.push(constant);
}

void yupc::BoolCodegen(bool value) 
{
    llvm::IntegerType *type = llvm::Type::getInt8Ty(*yupc::CompilationUnits.back()->Context);
    llvm::Constant *constant = llvm::ConstantInt::get(type, value);

    yupc::CompilationUnits.back()->ValueStack.push(constant);
}

void yupc::CharCodegen(std::string text) 
{

    char *cstr = new char[text.length() + 1];
    strcpy(cstr, &text.c_str()[1]);

    llvm::IntegerType *type = llvm::Type::getInt8Ty(*yupc::CompilationUnits.back()->Context);
    llvm::Constant *constant = llvm::ConstantInt::get(type, *cstr);

    yupc::CompilationUnits.back()->ValueStack.push(constant);

    delete []cstr;
}

void yupc::StringCodegen(std::string text) 
{
    llvm::Constant *gstrptr = yupc::CompilationUnits.back()->IRBuilder->CreateGlobalStringPtr(llvm::StringRef(text));
    yupc::CompilationUnits.back()->ValueStack.push(gstrptr);
}

void yupc::NullCodegen() 
{
    llvm::Type *voidPtrType = llvm::Type::getInt8Ty(*yupc::CompilationUnits.back()->Context)->getPointerTo();
    llvm::Constant *nullp = llvm::ConstantPointerNull::getNullValue(voidPtrType);

    yupc::CompilationUnits.back()->ValueStack.push(nullp);
}

std::any yupc::Visitor::visitConstant(yupc::YupParser::ConstantContext *ctx) 
{
    
    if (ctx->ValueInteger() != nullptr) 
    {
        std::string text = ctx->ValueInteger()->getText();

        int64_t value = yupc::StringToInt64(text);
        yupc::IntegerCodegen(value);

        return nullptr;
    }

    if (ctx->ValueFloat() != nullptr) 
    {
        std::string text = ctx->ValueFloat()->getText();
        double value = std::atof(text.c_str());

        yupc::FloatCodegen(value);

        return nullptr;
    }

    if (ctx->ValueBool() != nullptr) 
    {
        std::string text = ctx->ValueBool()->getText();

        bool value = text == "True";
        yupc::BoolCodegen(value);

        return nullptr;
    }

    if (ctx->ValueChar() != nullptr) 
    {
        std::string text = ctx->ValueChar()->getText();

        yupc::CharCodegen(text);

        return nullptr;
    }

    if (ctx->ValueString() != nullptr) 
    {
        std::string text = ctx->ValueString()->getText();
        yupc::StringStripQuotes(text);

        yupc::StringCodegen(text);

        return nullptr;
    }

    if (ctx->ValueNull() != nullptr) 
    {
        yupc::NullCodegen();

        return nullptr;
    }

    yupc::GlobalLogger.LogCompilerError(ctx->start->getLine(), ctx->start->getCharPositionInLine(), 
                                        "couldn't match type and create a constant", 
                                        ctx->getText(), yupc::CompilationUnits.back()->SourceFile);
    exit(1);
    return nullptr;
}
