#include "compiler/Visitor.h"
#include "compiler/Type.h"
#include "compiler/Function.h"
#include "compiler/CompilationUnit.h"
#include "compiler/CodeBlock.h"
#include "Logger.h"

#include "parser/YupParser.h"

#include "llvm/IR/Value.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Comdat.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <any>
#include <cstddef>
#include <iostream>
#include <vector>

#define MAIN_FUNC_NAME "main"

yupc::FunctionParameter::FunctionParameter(llvm::Type  *_parameterType,
                                        std::string     _parameterName,
                                        bool            _isVarArg)

:   ParameterType   (_parameterType),
    ParameterName   (_parameterName),
    IsVarArg        (_isVarArg) { }


void yupc::FunctionDefinitionCodegen(llvm::Function *function) 
{
    llvm::BasicBlock *block = llvm::BasicBlock::Create(yupc::CompilationUnits.back()->Module->getContext(), "entry", function);
    yupc::CompilationUnits.back()->IRBuilder->SetInsertPoint(block);

    size_t len = function->arg_size();
    for (size_t i = 0; i < len; ++i) 
    {
        llvm::Argument &arg = *function->getArg(i);

        llvm::AllocaInst *alloca = yupc::CompilationUnits.back()->IRBuilder->CreateAlloca(arg.getType(), 0, arg.getName());
        yupc::CompilationUnits.back()->SymbolTable.back()[arg.getName().str()] = alloca;
        yupc::CompilationUnits.back()->IRBuilder->CreateStore(&arg, alloca);
    }
}

void yupc::FunctionCallCodegen(std::string functionName, std::vector<llvm::Value*> args, 
                                size_t line, size_t pos, std::string text) 
{
    llvm::Function *function = yupc::CompilationUnits.back()->Module->getFunction(functionName);

    if (function == nullptr) 
    {
        yupc::GlobalLogger.LogCompilerError(line, pos, "tried to call function " + functionName 
                    + " but it isn't declared", text, yupc::CompilationUnits.back()->SourceFile);
        exit(1);
    }

    bool is_void = function->getReturnType()->isVoidTy();
    if (is_void) 
    {
        llvm::CallInst *result = yupc::CompilationUnits.back()->IRBuilder->CreateCall(function, args, "");
        yupc::CompilationUnits.back()->ValueStack.push(result);

    } 
    else 
    {
        llvm::CallInst *result = yupc::CompilationUnits.back()->IRBuilder->CreateCall(function, args);
        yupc::CompilationUnits.back()->ValueStack.push(result);
    }
}

void yupc::FunctionSignatureCodegen(bool isVarArg, bool isPublic, std::string name, 
            llvm::Type *return_type, std::vector<yupc::FunctionParameter*> params) 
{
    std::vector<llvm::Type*> paramTypes;
    for (size_t i = 0; i < params.size(); ++i)
    {
        paramTypes.push_back(params[i]->ParameterType);
    }

    llvm::FunctionType *functionType = llvm::FunctionType::get(return_type, paramTypes, isVarArg);
    llvm::GlobalValue::LinkageTypes linkage_type = isPublic || name == MAIN_FUNC_NAME 
                                                    ? llvm::GlobalValue::ExternalLinkage 
                                                    : llvm::GlobalValue::PrivateLinkage;

    llvm::Function *function = llvm::Function::Create(functionType, linkage_type, 
                                                    name, yupc::CompilationUnits.back()->Module);

    size_t arg_max = function->arg_size();
    for (size_t i = 0; i < arg_max; ++i) 
    {
        function->getArg(i)->setName(params[i]->ParameterName);
    }

    yupc::CompilationUnits.back()->Functions[name] = function;
}

std::any yupc::Visitor::visitFunctionReturn(yupc::YupParser::FunctionReturnContext *ctx) 
{
    return this->visit(ctx->expression());
}

std::any yupc::Visitor::visitFunctionSignature(yupc::YupParser::FunctionSignatureContext *ctx) 
{
    std::string name = ctx->Identifier()->getText();

    this->visit(ctx->typeName());
    llvm::Type *return_type = yupc::CompilationUnits.back()->TypeStack.top();

    std::vector<yupc::FunctionParameter*> params;
    size_t paramsSize = ctx->functionParameterList() == nullptr 
                        ? 0
                        : ctx->functionParameterList()->functionParameter().size();
    for (size_t i = 0; i < paramsSize; ++i)
    {
        yupc::YupParser::FunctionParameterContext *p = ctx->functionParameterList()->functionParameter(i);
        yupc::FunctionParameter *fp = std::any_cast<yupc::FunctionParameter*>(this->visit(p));
        params.push_back(fp);
    }

    /*std::vector<llvm::Type*> paramTypes;
    size_t i = 0;
    while (i < params.size() && !params[i]->isVarArg)
    {
        paramTypes.push_back(params[i]->ParameterType);
        i++;
    }*/

    bool isPublic = ctx->KeywordPublic() != nullptr;
    bool isVarArg = !params.empty() && params.back()->IsVarArg;
    yupc::FunctionSignatureCodegen(isVarArg, isPublic, name, return_type, params);

    return nullptr;
}

std::any yupc::Visitor::visitFunctionParameter(yupc::YupParser::FunctionParameterContext *ctx) 
{
    if (ctx->Identifier() != nullptr)
    {
        std::string name = ctx->Identifier()->getText();    

        this->visit(ctx->typeAnnotation());
        llvm::Type *resolvedType = yupc::CompilationUnits.back()->TypeStack.top();

        yupc::FunctionParameter *funcParam = new yupc::FunctionParameter(resolvedType, name, false);

        return funcParam;
    }
    else
    {
        yupc::FunctionParameter *funcParam = new yupc::FunctionParameter(nullptr, "", true);

        return funcParam;
    }
}

std::any yupc::Visitor::visitFunctionDefinition(yupc::YupParser::FunctionDefinitionContext *ctx) 
{
    std::string func_name = ctx->functionSignature()->Identifier()->getText();

    this->visit(ctx->functionSignature());
    llvm::Function *function = yupc::CompilationUnits.back()->Functions[func_name];
    
    if (!function) 
    {
        yupc::GlobalLogger.LogCompilerError(ctx->start->getLine(), ctx->start->getCharPositionInLine(), 
                                            "cannot resolve the signature for function " + func_name, 
                                            ctx->getText(), yupc::CompilationUnits.back()->SourceFile);
        exit(1);
    }

    yupc::CreateNewScope();
    yupc::FunctionDefinitionCodegen(function);

    bool is_void = function->getReturnType()->isVoidTy();
    if (!is_void) 
    {
        this->visit(ctx->codeBlock());
        llvm::Value *returnValue = yupc::CompilationUnits.back()->ValueStack.top();
        yupc::CompilationUnits.back()->IRBuilder->CreateRet(returnValue);
        yupc::CompilationUnits.back()->ValueStack.pop();
    } 
    else 
    {
        this->visit(ctx->codeBlock());
        yupc::CompilationUnits.back()->IRBuilder->CreateRetVoid();
    }

    yupc::DropScope();
    verifyFunction(*function, &llvm::errs());

    return nullptr;
}

std::any yupc::Visitor::visitFunctionCall(yupc::YupParser::FunctionCallContext *ctx) 
{
    std::string functionName = ctx->Identifier()->getText();

    auto find = yupc::CompilationUnits.back()->SymbolTable.back().find(functionName);
    auto end  = yupc::CompilationUnits.back()->SymbolTable.back().end();

    if (find != end) 
    {
        yupc::GlobalLogger.LogCompilerError(ctx->start->getLine(), ctx->start->getCharPositionInLine(), 
            "cannot call function \"" + functionName + "\" because it doesn't exist in the symbol table",
            ctx->getText(), yupc::CompilationUnits.back()->SourceFile);
        exit(1);
    }

    std::vector<llvm::Value*> args;
    size_t exprLength = ctx->functionCallArgList()->expression().size();
    for (size_t i = 0; i < exprLength; ++i) 
    {
        yupc::YupParser::ExpressionContext *expr = ctx->functionCallArgList()->expression()[i];

        this->visit(expr);
        llvm::Value *argValue = yupc::CompilationUnits.back()->ValueStack.top();
        args.push_back(argValue);
        yupc::CompilationUnits.back()->ValueStack.pop();
    }

    yupc::FunctionCallCodegen(functionName, args, ctx->start->getLine(), 
                    ctx->start->getCharPositionInLine(), ctx->getText());
    
    return nullptr;
}
