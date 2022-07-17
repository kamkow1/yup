#include <visitor.h>
#include <util.h>
#include <messaging/errors.h>
#include <compiler/function.h>
#include <compiler/type.h>

using namespace llvm;

FuncParam::FuncParam(llvm::Type* type, std::string pn)
{
    this->paramType = type;
    this->paramName = std::move(pn);
}


std::any Visitor::visitFunc_return(YupParser::Func_returnContext *ctx)
{
    std::any value = this->visit(ctx->expr());
    return value;
}

std::any Visitor::visitFunc_def(YupParser::Func_defContext *ctx)
{
    std::string funcName = ctx
            ->func_signature()
            ->IDENTIFIER()
            ->getText();

    llvm::Function* function = std::any_cast<llvm::Function*>(this->visit(ctx->func_signature()));
    
    if (!function)
    {
        std::string errorMessage = "cannot resolve the signature for function " + funcName;
        logCompilerError(errorMessage);
        exit(1);
    }

    llvm::BasicBlock* block = llvm::BasicBlock::Create(context, "entry", function);
    irBuilder.SetInsertPoint(block);

    symbolTable.clear();

    for (auto &arg : function->args())
    {
        AllocaInst *alloca = irBuilder.CreateAlloca(arg.getType(), nullptr, arg.getName());

        symbolTable[arg.getName().str()];
    }


    if (!function->getFunctionType()->getReturnType()->isVoidTy())
    {
        this->visit(ctx->code_block());

        llvm::Value* retValue = valueStack.top();
        irBuilder.CreateRet(retValue);
        valueStack.pop();
    }
    else
    {
        this->visit(ctx->code_block());
        irBuilder.CreateRetVoid();
    }

    llvm::verifyFunction(*function, &llvm::outs());

    return function;
}

std::any Visitor::visitFunc_signature(YupParser::Func_signatureContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();
    TypeAnnotation typeAnnot = 
        std::any_cast<TypeAnnotation>(this->visit(ctx->type_annot()));
    llvm::Type* returnType = resolveType(typeAnnot.typeName);

    std::vector<FuncParam*> params;
    for (YupParser::Func_paramContext* const p : ctx->func_param())
    {
        FuncParam* fp = std::any_cast<FuncParam*>(this->visit(p));
        params.push_back(fp);
    }

    std::vector<llvm::Type*> paramTypes;
    for (const FuncParam* pt : params)
    {
        paramTypes.push_back(pt->paramType);
    }

    llvm::FunctionType *functionType = llvm::FunctionType::get(
            returnType,
            paramTypes,
            false);

    llvm::Function* function = llvm::Function::Create(
            functionType,
            llvm::Function::ExternalLinkage,
            name,
            module.get());

    int argMax = function->arg_size();
    for (int i = 0; i < argMax; ++i)
    {
        std::string pn = params[i]->paramName;
        function->getArg(i)->setName(pn);
    }

    return function;
}

std::any Visitor::visitFunc_param(YupParser::Func_paramContext *ctx)
{
    TypeAnnotation typeAnnot = std::any_cast<TypeAnnotation>(this->visit(ctx->type_annot()));
    llvm::Type* resolvedType = resolveType(typeAnnot.typeName);

    // debug
    std::string typeStr;
    llvm::raw_string_ostream rso(typeStr);
    resolvedType->print(rso);
    std::cout << "type: " << rso.str() << "\n";

    std::string name = ctx->IDENTIFIER()->getText();

    FuncParam* funcParam = new FuncParam(resolvedType, name);
    return funcParam;
}

std::any Visitor::visitFunc_call(YupParser::Func_callContext *ctx)
{
    std::string funcName = ctx->IDENTIFIER()->getText();

    if (symbolTable.find(funcName) != symbolTable.end())
    {
        std::string errorMessage
            = "cannot call function \"" + funcName + "\" because it doesn't exist in the symbol table";
        logCompilerError(errorMessage);
        exit(1);
    }

    std::vector<llvm::Value*> args;
    int exprLength = ctx->expr().size();
    for (unsigned i = 0; i < exprLength; ++i)
    {
        YupParser::ExprContext* expr = ctx->expr()[i];
        this->visit(expr);
        llvm::Value* argVal = valueStack.top();
        args.push_back(argVal);
        valueStack.pop();
    }

    llvm::Function* fnCallee = module->getFunction(funcName);

    if (args.size() != fnCallee->arg_size())
    {
        std::string errorMessage= "found function \"" + funcName
                + "\" but couldn't match given argument list length to the function signature";
        logCompilerError(errorMessage);
        exit(1);
    }

    std::string callLabel = std::string("call") + "_" + funcName;

    auto result = irBuilder.CreateCall(fnCallee, args, callLabel);
    valueStack.push(result);
    return nullptr;
}