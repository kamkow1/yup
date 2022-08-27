#include "Compiler/Visitor.h"
#include "Compiler/CompilationUnit.h"
#include "Compiler/Operator.h"

#include "Parser/YupParser.h"

#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"

#include <cstddef>
#include <iostream>
#include <any>
#include <vector>
#include <map>
#include <string>

yupc::OperatorArgument::OperatorArgument(llvm::Value *_operatorPossibleValue,
                                        llvm::Type *_operatorPossibleType)
:   OperatorPossibleValue(_operatorPossibleValue),
    OperatorPossibleType(_operatorPossibleType)
{

}

llvm::Value *SizeOfOperator(std::vector<yupc::OperatorArgument*> &args) 
{
    yupc::OperatorArgument *baseSize = args[0];
    llvm::Constant *size = llvm::ConstantExpr::getSizeOf(baseSize->OperatorPossibleType);

    return size;
}

llvm::Type *TypeOfOperator(std::vector<yupc::OperatorArgument*> &args)
{
    yupc::OperatorArgument *baseSize = args[0];
    llvm::Type *type = baseSize->OperatorPossibleValue->getType();

    return type;
}


std::map<std::string, std::function<llvm::Value*(std::vector<yupc::OperatorArgument*>&)>> yupc::ValueOperators
{
    { "sizeof", &SizeOfOperator },
};

std::map<std::string, std::function<llvm::Type*(std::vector<yupc::OperatorArgument*>&)>> yupc::TypeOperators
{
    { "typeof", &TypeOfOperator }
};

std::any yupc::Visitor::visitOperator(yupc::YupParser::OperatorContext *ctx) 
{

    std::string operatorName = ctx->Identifier()->getText();

    std::vector<yupc::OperatorArgument*> args;
    for (size_t i = 0; i < ctx->expression().size(); i++) 
    {

        yupc::OperatorArgument *operArg;

        if (dynamic_cast<yupc::YupParser::TypeNameExpressionContext*>(ctx->expression(i)) != nullptr) 
        {
            this->visit(ctx->expression(i));
            operArg = new yupc::OperatorArgument(nullptr, yupc::CompilationUnits.back()->TypeStack.top());
        } 
        else if (dynamic_cast<yupc::YupParser::IdentifierExpressionContext*>(ctx->expression(i)) != nullptr)
        {
            this->visit(ctx->expression(i));
            operArg = new yupc::OperatorArgument(yupc::CompilationUnits.back()->ValueStack.top(), nullptr);
        }
        else
        {
            std::any result = this->visit(ctx->expression(i));
            try 
            {
                llvm::Value *v = std::any_cast<llvm::Value*>(result);
                operArg->OperatorPossibleValue = v;
            } 
            catch (std::bad_any_cast) { }

            try 
            {
                llvm::Type *t = std::any_cast<llvm::Type*>(result);
                operArg->OperatorPossibleType = t;
            } 
            catch (std::bad_any_cast) {}
        }

        args.push_back(operArg);
    }

    if (yupc::ValueOperators.contains(operatorName))
    {
        llvm::Value *value = yupc::ValueOperators[operatorName](args);
        yupc::CompilationUnits.back()->ValueStack.push(value);
        return value;
    }
    else
    {
        llvm::Type *type = yupc::TypeOperators[operatorName](args);
        yupc::CompilationUnits.back()->TypeStack.push(type);
        return type;
    }

    return nullptr;
}
