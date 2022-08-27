#include "Compiler/Visitor.h"
#include "Compiler/BinaryOperation.h"
#include "Compiler/CompilationUnit.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/InstrTypes.h"

#include <cstring>
#include <cfloat>
#include <limits.h>
#include <string>

llvm::Value *yupc::AddCodegen(llvm::Value *lhs, llvm::Value *rhs)
{
    llvm::Value *result;

    if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy())
    {
        result = yupc::CompilationUnits.back()->IRBuilder->CreateAdd(lhs, rhs);
    }
    else if (lhs->getType()->isFloatTy() && rhs->getType()->isFloatTy())
    {
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFAdd(lhs, rhs);
    }
    else if (lhs->getType()->isFloatTy() && rhs->getType()->isIntegerTy())
    {
        llvm::Type *fpType = llvm::Type::getFloatTy(*yupc::CompilationUnits.back()->Context);
        llvm::Value *cast = yupc::CompilationUnits.back()->IRBuilder->CreateSIToFP(rhs, fpType);
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFAdd(lhs, cast);
    }
    else if (lhs->getType()->isIntegerTy() && rhs->getType()->isFloatTy())
    {
        llvm::Type *fpType = llvm::Type::getFloatTy(*yupc::CompilationUnits.back()->Context);
        llvm::Value *cast = yupc::CompilationUnits.back()->IRBuilder->CreateSIToFP(lhs, fpType);
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFAdd(cast, rhs);
    }

    return result;
}

llvm::Value *yupc::SubCodegen(llvm::Value *lhs, llvm::Value *rhs)
{
    llvm::Value *result;

    if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy())
    {
        result = yupc::CompilationUnits.back()->IRBuilder->CreateSub(lhs, rhs);
    }
    else if (lhs->getType()->isFloatTy() && rhs->getType()->isFloatTy())
    {
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFSub(lhs, rhs);
    }
    else if (lhs->getType()->isFloatTy() && rhs->getType()->isIntegerTy())
    {
        llvm::Type *fpType = llvm::Type::getFloatTy(*yupc::CompilationUnits.back()->Context);
        llvm::Value *cast = yupc::CompilationUnits.back()->IRBuilder->CreateSIToFP(rhs, fpType);
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFSub(lhs, cast);
    }
    else if (lhs->getType()->isIntegerTy() && rhs->getType()->isFloatTy())
    {
        llvm::Type *fpType = llvm::Type::getFloatTy(*yupc::CompilationUnits.back()->Context);
        llvm::Value *cast = yupc::CompilationUnits.back()->IRBuilder->CreateSIToFP(lhs, fpType);
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFSub(cast, rhs);
    }

    return result;
}

llvm::Value *yupc::MulCodegen(llvm::Value *lhs, llvm::Value *rhs)
{
    llvm::Value *result;

    if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy())
    {
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFMul(lhs, rhs);
    }
    else if (lhs->getType()->isFloatTy() && rhs->getType()->isFloatTy())
    {
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFMul(lhs, rhs);
    }
    else if (lhs->getType()->isFloatTy() && rhs->getType()->isIntegerTy())
    {
        llvm::Type *fpType = llvm::Type::getFloatTy(*yupc::CompilationUnits.back()->Context);
        llvm::Value *cast = yupc::CompilationUnits.back()->IRBuilder->CreateSIToFP(rhs, fpType);
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFMul(lhs, cast);
    }
    else if (lhs->getType()->isIntegerTy() && rhs->getType()->isFloatTy())
    {
        llvm::Type *fpType = llvm::Type::getFloatTy(*yupc::CompilationUnits.back()->Context);
        llvm::Value *cast = yupc::CompilationUnits.back()->IRBuilder->CreateSIToFP(lhs, fpType);
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFMul(cast, rhs);
    }

    return result;
}

llvm::Value *yupc::DivCodgen(llvm::Value *lhs, llvm::Value *rhs)
{
    llvm::Value *result;

    if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy())
    {
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFDiv(lhs, rhs);
    }
    else if (lhs->getType()->isFloatTy() && rhs->getType()->isFloatTy())
    {
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFDiv(lhs, rhs);
    }
    else if (lhs->getType()->isFloatTy() && rhs->getType()->isIntegerTy())
    {
        llvm::Type *fpType = llvm::Type::getFloatTy(*yupc::CompilationUnits.back()->Context);
        llvm::Value *cast = yupc::CompilationUnits.back()->IRBuilder->CreateSIToFP(rhs, fpType);
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFDiv(lhs, cast);
    }
    else if (lhs->getType()->isIntegerTy() && rhs->getType()->isFloatTy())
    {
        llvm::Type *fpType = llvm::Type::getFloatTy(*yupc::CompilationUnits.back()->Context);
        llvm::Value *cast = yupc::CompilationUnits.back()->IRBuilder->CreateSIToFP(lhs, fpType);
        result = yupc::CompilationUnits.back()->IRBuilder->CreateFDiv(cast, rhs);
    }

    return result;
}

llvm::Value *yupc::BinaryOparationCodegen(llvm::Value *lhs, llvm::Value *rhs, std::string op) 
{

    llvm::Value *math_inst;

    char *cstr = new char[op.length() - 1];
    strcpy(cstr, op.c_str());

    switch (*cstr) 
    {
        case '+':
            math_inst = yupc::AddCodegen(lhs, rhs);
            break;
            
        case '-':
            math_inst = yupc::SubCodegen(lhs, rhs);
            break;

        case '*':
            math_inst = yupc::MulCodegen(lhs, rhs);
            break;

        case '/': 
            math_inst = yupc::DivCodgen(lhs, rhs);
            break;
    }

    delete []cstr;
    
    return math_inst;
}

std::any yupc::Visitor::visitBinaryOperationExpression(yupc::YupParser::BinaryOperationExpressionContext *ctx) 
{
    std::string op = ctx->binaryOperator()->getText();

    this->visit(ctx->expression(0));
    llvm::Value *lhs = yupc::CompilationUnits.back()->ValueStack.top();

    this->visit(ctx->expression(1));
    llvm::Value *rhs = yupc::CompilationUnits.back()->ValueStack.top();

    yupc::CompilationUnits.back()->ValueStack.pop();
    yupc::CompilationUnits.back()->ValueStack.pop();

    llvm::Value *result = yupc::BinaryOparationCodegen(lhs, rhs, op);
    yupc::CompilationUnits.back()->ValueStack.push(result);

    return nullptr;
}
