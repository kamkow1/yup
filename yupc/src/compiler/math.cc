#include <compiler/visitor.h>
#include <compiler/math.h>
#include <compiler/compilation_unit.h>

#include <llvm/IR/Constants.h>

#include <cstring>
#include <cfloat>
#include <limits.h>

using namespace llvm;
using namespace yupc;

namespace cv = compiler::visitor;
namespace cm = compiler::math;
namespace com_un = compiler::compilation_unit;

void cm::math_oper_expr_codegen(Value *lhs, Value *rhs, std::string op) {

    Value *math_inst;

    auto *cstr = new char[op.length() - 1];
    strcpy(cstr, op.c_str());

    switch (*cstr) {
        case '+': 
            math_inst = com_un::comp_units.back()->ir_builder->CreateAdd(lhs, rhs);
            break;

        case '-':
            math_inst = com_un::comp_units.back()->ir_builder->CreateSub(lhs, rhs);
            break;

        case '*':
            math_inst = com_un::comp_units.back()->ir_builder->CreateMul(lhs, rhs);
            break;

        case '/': 
            math_inst = com_un::comp_units.back()->ir_builder->CreateFDiv(lhs, rhs);
            break;
    }

    delete []cstr;
    
    com_un::comp_units.back()->value_stack.push(math_inst);
}

std::any cv::Visitor::visitMathOperExpr(parser::YupParser::MathOperExprContext *ctx) {

    auto op = ctx->binop()->getText();

    this->visit(ctx->expr(0));
    auto *lhs = com_un::comp_units.back()->value_stack.top();

    this->visit(ctx->expr(1));
    auto *rhs = com_un::comp_units.back()->value_stack.top();

    cm::math_oper_expr_codegen(lhs, rhs, op);

    return nullptr;
}
