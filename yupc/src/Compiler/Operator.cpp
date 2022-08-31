#include "Compiler/Visitor.h"
#include "Compiler/CompilationUnit.h"
#include "Compiler/Operator.h"
#include "utils.h"

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

llvm::Value *SizeOfTypeOperator(std::vector<yupc::OperatorArgument*> &args) 
{
    yupc::OperatorArgument *baseSize = args[0];
    llvm::Constant *size = llvm::ConstantExpr::getSizeOf(baseSize->OperatorPossibleType);

    return size;
}

llvm::Value *SizeOfExprOperator(std::vector<yupc::OperatorArgument*> &args) 
{
    yupc::OperatorArgument *baseSize = args[0];
    llvm::Constant *size = llvm::ConstantExpr::getSizeOf(baseSize->OperatorPossibleValue->getType());

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
    { "SizeOfType", &SizeOfTypeOperator },
    { "SizeOfExpr", &SizeOfExprOperator }
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
        size_t ts_current = yupc::CompilationUnits.back()->TypeStack.size();
        this->visit(ctx->expression(i));
        if (ts_current != yupc::CompilationUnits.back()->TypeStack.size())
        {
            operArg = new yupc::OperatorArgument(nullptr, yupc::CompilationUnits.back()->TypeStack.top());
        } 
        else
        {
            operArg = new yupc::OperatorArgument(yupc::CompilationUnits.back()->ValueStack.top(), nullptr);
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
