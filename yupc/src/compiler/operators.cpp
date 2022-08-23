#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>
#include <compiler/operators.h>

#include <cstddef>
#include <parser/YupParser.h>

#include <llvm/IR/Type.h>
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

llvm::Value *sizeof_oper(std::vector<yupc::OperArg*> &args) 
{
    yupc::OperArg *base_sz = args[0];
    llvm::Constant *size = llvm::ConstantExpr::getSizeOf(base_sz->oper_type);

    return size;
}

llvm::Type *typeof_oper(std::vector<yupc::OperArg*> &args)
{
    yupc::OperArg *base_sz = args[0];
    llvm::Type *type = base_sz->oper_value->getType();

    return type;
}


std::map<std::string, std::function<llvm::Value*(std::vector<yupc::OperArg*>&)>> yupc::value_operators 
{
    { "sizeof", &sizeof_oper },
};

std::map<std::string, std::function<llvm::Type*(std::vector<yupc::OperArg*>&)>> yupc::type_operators 
{
    { "typeof", &typeof_oper }
};

std::any yupc::Visitor::visitOperator(yupc::YupParser::OperatorContext *ctx) 
{

    std::string oper_name = ctx->IDENTIFIER()->getText();

    std::vector<yupc::OperArg*> args;
    for (size_t i = 0; i < ctx->expr().size(); i++) 
    {

        yupc::OperArg *oa_un = new yupc::OperArg;

        if (dynamic_cast<yupc::YupParser::TypeNameExprContext*>(ctx->expr(i)) != nullptr) 
        {
            this->visit(ctx->expr(i));
            oa_un->oper_type = yupc::comp_units.back()->type_stack.top();            
        } 
        else if (dynamic_cast<yupc::YupParser::IdentifierExprContext*>(ctx->expr(i)) != nullptr)
        {
            this->visit(ctx->expr(i));
            oa_un->oper_value = yupc::comp_units.back()->value_stack.top();
        }
        else
        {
            std::cout << "oper\n";
            std::any result = this->visit(ctx->expr(i));
            try 
            {
                llvm::Value *v = std::any_cast<llvm::Value*>(result);
                oa_un->oper_value = v;
            } 
            catch (std::bad_any_cast) { }

            try 
            {
                llvm::Type *t = std::any_cast<llvm::Type*>(result);
                oa_un->oper_type = t;
            } 
            catch (std::bad_any_cast) {}
        }

        args.push_back(oa_un);
    }

    if (value_operators.contains(oper_name))
    {
        llvm::Value *value = value_operators[oper_name](args);
        yupc::comp_units.back()->value_stack.push(value);
        return value;
    }
    else
    {
        llvm::Type *type = type_operators[oper_name](args);
        yupc::comp_units.back()->type_stack.push(type);
        return type;
    }

    return nullptr;
}
