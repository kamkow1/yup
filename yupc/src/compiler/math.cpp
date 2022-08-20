#include <compiler/visitor.h>
#include <compiler/math.h>
#include <compiler/compilation_unit.h>

#include <llvm/IR/Constants.h>
#include <llvm/IR/Value.h>

#include <cstring>
#include <cfloat>
#include <limits.h>
#include <string>

void yupc::math_oper_expr_codegen(llvm::Value *lhs, llvm::Value *rhs, std::string op) 
{

    llvm::Value *math_inst;

    char *cstr = new char[op.length() - 1];
    strcpy(cstr, op.c_str());

    switch (*cstr) 
    {
        case '+': 
            math_inst = yupc::comp_units.back()->ir_builder->CreateAdd(lhs, rhs);
            break;

        case '-':
            math_inst = yupc::comp_units.back()->ir_builder->CreateSub(lhs, rhs);
            break;

        case '*':
            math_inst = yupc::comp_units.back()->ir_builder->CreateMul(lhs, rhs);
            break;

        case '/': 
            math_inst = yupc::comp_units.back()->ir_builder->CreateFDiv(lhs, rhs);
            break;
    }

    delete []cstr;
    
    yupc::comp_units.back()->value_stack.push(math_inst);
}

std::any yupc::Visitor::visitMathOperExpr(yupc::YupParser::MathOperExprContext *ctx) 
{
    std::string op = ctx->binop()->getText();

    this->visit(ctx->expr(0));
    llvm::Value *lhs = yupc::comp_units.back()->value_stack.top();

    this->visit(ctx->expr(1));
    llvm::Value *rhs = yupc::comp_units.back()->value_stack.top();

    yupc::math_oper_expr_codegen(lhs, rhs, op);

    return nullptr;
}
