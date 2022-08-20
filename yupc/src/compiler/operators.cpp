#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>
#include <compiler/operators.h>

#include <parser/YupParser.h>

#include <llvm/IR/Constants.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Value.h>

#include <iostream>
#include <any>
#include <vector>
#include <map>
#include <string>

llvm::Value *size_oper(std::vector<yupc::OperArg*> args) 
{
    yupc::OperArg *base_sz = args[0];
    llvm::Constant *size = llvm::ConstantExpr::getSizeOf(base_sz->oper_type);

    return size;
}


std::map<std::string, std::function<llvm::Value *(std::vector<yupc::OperArg *>)>> yupc::opers 
{
    { "size", &size_oper }
};

std::any yupc::Visitor::visitOperator(yupc::YupParser::OperatorContext *ctx) 
{

    auto oper_name = ctx->IDENTIFIER()->getText();

    std::vector<yupc::OperArg*> args;
    for (yupc::YupParser::ExprContext *aa : ctx->expr()) 
    {

        auto *oa_un = new yupc::OperArg;

        if (dynamic_cast<yupc::YupParser::TypeNameExprContext*>(aa) != nullptr) 
        {
            this->visit(aa);

            oa_un->oper_type = yupc::comp_units.back()->type_stack.top();
            args.push_back(oa_un);
        } 
        else 
        {
            this->visit(aa);

            oa_un->oper_value = yupc::comp_units.back()->value_stack.top();
            args.push_back(oa_un);
        }
    }

    std::function<llvm::Value *(std::vector<yupc::OperArg *>)> oper = yupc::opers[oper_name];
    auto *result = oper(args);

    yupc::comp_units.back()->value_stack.push(result);

    return nullptr;
}
