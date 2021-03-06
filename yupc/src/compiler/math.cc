#include "compiler/visitor.h"
#include "compiler/math.h"

#include "llvm/IR/Constants.h"

#include "cstring"
#include "cfloat"
#include "limits.h"

using namespace llvm;
using namespace yupc;

namespace cv = compiler::visitor;
namespace cm = compiler::math;

void cm::math_oper_expr_codegen(Value *lhs, Value *rhs, std::string op)
{
    Value *math_inst;

    char *cstr = new char[op.length() - 1];
    strcpy(cstr, op.c_str());

    switch (*cstr) 
    {
        case '+': 
            math_inst = cv::ir_builder.CreateAdd(lhs, rhs);
            break;

        case '-':
            math_inst = cv::ir_builder.CreateSub(lhs, rhs);
            break;

        case '*':
            math_inst = cv::ir_builder.CreateMul(lhs, rhs);
            break;

        case '/': 
            math_inst = cv::ir_builder.CreateFDiv(lhs, rhs);
            break;
    }

    delete []cstr;
    
    cv::value_stack.push(math_inst);
}

std::any cv::Visitor::visitMathOperExpr(parser::YupParser::MathOperExprContext *ctx)
{
    std::string op = ctx->binop()->getText();

    this->visit(ctx->expr(0));
    Value *lhs = value_stack.top();

    this->visit(ctx->expr(1));
    Value *rhs = value_stack.top();

    cm::math_oper_expr_codegen(lhs, rhs, op);

    return nullptr;
}
