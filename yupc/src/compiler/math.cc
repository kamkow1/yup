#include "compiler/visitor.h"
#include "compiler/math.h"
#include "cstring"
#include "llvm/IR/Constants.h"
#include <cfloat>
#include "limits.h"

using namespace llvm;

void mathOperExpr_codegen(Value *lhs, Value *rhs, std::string op)
{
    Value *mathInst;

    char *cstr = new char[op.length() - 1];
    strcpy(cstr, op.c_str());

    switch (*cstr) 
    {
        case '+': 
            mathInst = irBuilder.CreateAdd(lhs, rhs);
            break;

        case '-':
            mathInst = irBuilder.CreateSub(lhs, rhs);
            break;

        case '*':
            mathInst = irBuilder.CreateMul(lhs, rhs);
            break;

        case '/': 
            mathInst = irBuilder.CreateFDiv(lhs, rhs);
            break;
    }

    delete []cstr;
    
    valueStack.push(mathInst);
}

std::any Visitor::visitMathOperExpr(YupParser::MathOperExprContext *ctx)
{
    std::string op = ctx->binop()->getText();

    this->visit(ctx->expr(0));
    Value *lhs = valueStack.top();

    this->visit(ctx->expr(1));
    Value *rhs = valueStack.top();

    mathOperExpr_codegen(lhs, rhs, op);

    return nullptr;
}
