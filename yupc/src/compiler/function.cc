#include "visitor.h"
#include "util.h"
#include "llvm/IR/Type.h"
#include "string.h"
#include "messaging/errors.h"
#include "compiler/type.h"

using namespace llvm;

struct FuncParam
{
    Type *paramType;
    std::string paramName;
};

static std::map<std::string, Function*> functionTable;

void funcDef_codegen(Function *function)
{
    BasicBlock *block = BasicBlock::Create(context, "entry", function);
    irBuilder.SetInsertPoint(block);

    std::map<std::string, AllocaInst*> map;
    symbolTable.push(map);

    size_t len = function->arg_size();
    for (size_t i = 0; i < len; ++i)
    {
        Argument &arg = *function->getArg(i);

        AllocaInst *alloca = irBuilder.CreateAlloca(
            arg.getType(), 0, arg.getName());

        symbolTable.top()[arg.getName().str()] = alloca;
        irBuilder.CreateStore(&arg, alloca);
    }
}

void funcCall_codegen(std::string funcName, 
                    size_t exprLength, std::vector<Value*> args)
{
    Function *fnCallee = module->getFunction(funcName);


    if (args.size() != fnCallee->arg_size())
    {
        logCompilerError("found function \"" + funcName
            + "\" but couldn't match given argument" 
            + " list length to the function signature");
        exit(1);
    }


    bool isVoid = fnCallee->getReturnType()->isVoidTy();
    if (isVoid)
    {
        CallInst *result = irBuilder.CreateCall(fnCallee, args, "");
        valueStack.push(result);
    }
    else
    {
        std::string callLabel = std::string("call") + "_" + funcName;
        CallInst *result = irBuilder.CreateCall(fnCallee, args, callLabel);
        valueStack.push(result);
    }
}

void funcSig_codegen(std::string name, Type *returnType, 
    std::vector<Type*> paramTypes, std::vector<FuncParam*> params)
{
    FunctionType *functionType = FunctionType::get(
            returnType,
            paramTypes,
            false);

    Function *function = Function::Create(
            functionType,
            Function::ExternalLinkage,
            name,
            module.get());

    int argMax = function->arg_size();
    for (int i = 0; i < argMax; ++i)
    {
        std::string pn = params[i]->paramName;
        function->getArg(i)->setName(pn);
    }

    functionTable[name] = function;
}

std::any Visitor::visitFunc_return(YupParser::Func_returnContext *ctx)
{
    std::any value = this->visit(ctx->expr());
    return value;
}

std::any Visitor::visitFunc_signature(YupParser::Func_signatureContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();        
    Type *returnType = std::any_cast<Type*>(
        this->visit(ctx->type_annot()));

    std::vector<FuncParam*> params;
    for (YupParser::Func_paramContext *const p : ctx->func_param())
    {
        FuncParam *fp = std::any_cast<FuncParam*>(this->visit(p));
        params.push_back(fp);
    }

    std::vector<Type*> paramTypes;
    for (const FuncParam *pt : params)
    {
        paramTypes.push_back(pt->paramType);
    }

    funcSig_codegen(name, returnType, paramTypes, params);

    return nullptr;
}

std::any Visitor::visitFunc_param(YupParser::Func_paramContext *ctx)
{
    Type *resolvedType 
        = std::any_cast<Type*>(this->visit(ctx->type_annot()));

    std::string name = ctx->IDENTIFIER()->getText();

    FuncParam *funcParam = new FuncParam{resolvedType, name};
    return funcParam;
}

std::any Visitor::visitFunc_def(YupParser::Func_defContext *ctx)
{
    std::string funcName = ctx
            ->func_signature()
            ->IDENTIFIER()
            ->getText();

    this->visit(ctx->func_signature());
    Function *function = functionTable[funcName];
    
    if (!function)
    {
        logCompilerError("cannot resolve the signature for function " + funcName);
        exit(1);
    }

    funcDef_codegen(function);

    bool isVoid = function->getReturnType()->isVoidTy();
    if (!isVoid)
    {
        this->visit(ctx->code_block());

        Value *retValue = valueStack.top();
        irBuilder.CreateRet(retValue);
        valueStack.pop();
    }
    else
    {
        this->visit(ctx->code_block());
        irBuilder.CreateRetVoid();
    }

    verifyFunction(*function, &outs());
    symbolTable.pop();

    return nullptr;
}

std::any Visitor::visitFunc_call(YupParser::Func_callContext *ctx)
{
    std::string funcName = ctx->IDENTIFIER()->getText();

    if (symbolTable.top().find(funcName) != symbolTable.top().end())
    {
        logCompilerError("cannot call function \"" + funcName 
            + "\" because it doesn't exist in the symbol table");
        exit(1);
    }

    std::vector<Value*> args;
    size_t exprLength = ctx->expr().size();
    for (size_t i = 0; i < exprLength; ++i)
    {
        YupParser::ExprContext* expr = ctx->expr()[i];
        this->visit(expr);
        Value *argVal = valueStack.top();
        args.push_back(argVal);
        valueStack.pop();
    }

    funcCall_codegen(funcName, exprLength, args);
    
    return nullptr;
}
