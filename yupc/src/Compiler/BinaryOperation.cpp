#include "Compiler/Visitor.h"
#include "Compiler/BinaryOperation.h"
#include "Compiler/CompilationUnit.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Value.h"

#include <cstring>
#include <cfloat>
#include <limits.h>
#include <string>

void yupc::BinaryOparationCodegen(llvm::Value *lhs, llvm::Value *rhs, std::string op) 
{

    llvm::Value *math_inst;

    char *cstr = new char[op.length() - 1];
    strcpy(cstr, op.c_str());

    switch (*cstr) 
    {
        case '+': 
            math_inst = yupc::CompilationUnits.back()->IRBuilder->CreateAdd(lhs, rhs);
            break;

        case '-':
            math_inst = yupc::CompilationUnits.back()->IRBuilder->CreateSub(lhs, rhs);
            break;

        case '*':
            math_inst = yupc::CompilationUnits.back()->IRBuilder->CreateMul(lhs, rhs);
            break;

        case '/': 
            math_inst = yupc::CompilationUnits.back()->IRBuilder->CreateFDiv(lhs, rhs);
            break;
    }

    delete []cstr;
    
    yupc::CompilationUnits.back()->ValueStack.push(math_inst);
}

std::any yupc::Visitor::visitBinaryOperationExpression(yupc::YupParser::BinaryOperationExpressionContext *ctx) 
{
    std::string op = ctx->binaryOperator()->getText();

    this->visit(ctx->expression(0));
    llvm::Value *lhs = yupc::CompilationUnits.back()->ValueStack.top();

    this->visit(ctx->expression(1));
    llvm::Value *rhs = yupc::CompilationUnits.back()->ValueStack.top();

    yupc::BinaryOparationCodegen(lhs, rhs, op);
    yupc::CompilationUnits.back()->ValueStack.pop();
    yupc::CompilationUnits.back()->ValueStack.pop();

    return nullptr;
}
