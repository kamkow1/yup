#include "parser/YupParser.h"
#include <compiler/visitor.h>
#include <compiler/type.h>
#include <compiler/function.h>
#include <compiler/compilation_unit.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>
#include <msg/errors.h>
#include <util.h>

#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Comdat.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>

#include <string>
#include <any>
#include <cstddef>

#define MAIN_FUNC_NAME "main"

void yupc::func_def_codegen(llvm::Function *function) 
{
    llvm::BasicBlock *block = llvm::BasicBlock::Create(yupc::comp_units.back()->module->getContext(), "entry", function);

    yupc::comp_units.back()->ir_builder->SetInsertPoint(block);

    std::map<std::string, llvm::AllocaInst*> map;

    yupc::comp_units.back()->symbol_table.push_back(map);

    size_t len = function->arg_size();
    for (size_t i = 0; i < len; ++i) 
    {
        llvm::Argument &arg = *function->getArg(i);

        llvm::AllocaInst *alloca = yupc::comp_units.back()->ir_builder->CreateAlloca(arg.getType(), 0, arg.getName());

        yupc::comp_units.back()->symbol_table.back()[arg.getName().str()] = alloca;

        yupc::comp_units.back()->ir_builder->CreateStore(&arg, alloca);
    }
}

void yupc::func_call_codegen(std::string func_name, std::vector<llvm::Value*> args, std::string text) 
{
    llvm::Function *function = yupc::comp_units.back()->module->getFunction(func_name);

    if (function == nullptr) 
    {
        yupc::log_compiler_err("tried to call function " + func_name + " but it isn't declared", text);
        exit(1);
    }

    bool is_void = function->getFunctionType()->getReturnType()->isVoidTy();
    if (is_void) 
    {
        llvm::CallInst *result = yupc::comp_units.back()->ir_builder->CreateCall(function, args, "");

        for (size_t i = 0; i < function->arg_size(); i++) 
        {
            result->addParamAttr(i, llvm::Attribute::NoUndef);
        }

        yupc::comp_units.back()->value_stack.push(result);

    } 
    else 
    {
        llvm::CallInst *result = yupc::comp_units.back()->ir_builder->CreateCall(function, args);

        for (size_t i = 0; i < function->arg_size(); i++) 
        {
            result->addParamAttr(i, llvm::Attribute::NoUndef);
        }

        yupc::comp_units.back()->value_stack.push(result);
    }
}

void yupc::func_sig_codegen(bool is_ext, std::string name, llvm::Type *return_type, 
                            std::vector<llvm::Type*> param_types, std::vector<FuncParam*> params) 
{

    llvm::FunctionType *fn_type = llvm::FunctionType::get(return_type, param_types, false);

    llvm::GlobalValue::LinkageTypes linkage_type = is_ext || name == MAIN_FUNC_NAME 
                                                    ? llvm::GlobalValue::WeakAnyLinkage 
                                                    : llvm::GlobalValue::PrivateLinkage;

    llvm::Function *function = llvm::Function::Create(fn_type, linkage_type, name, yupc::comp_units.back()->module);

    for (size_t i = 0; i < function->arg_size(); i++) 
    {
        function->addParamAttr(i, llvm::Attribute::NoUndef);
    }

    function->setDSOLocal(!is_ext);

    size_t arg_max = function->arg_size();
    for (size_t i = 0; i < arg_max; ++i) 
    {
        std::string pn = params[i]->param_name;
        function->getArg(i)->setName(pn);
    }

    yupc::comp_units.back()->functions[name] = function;
}

std::any yupc::Visitor::visitFunc_return(yupc::YupParser::Func_returnContext *ctx) 
{
    std::any value = this->visit(ctx->expr());
    return value;
}

std::any yupc::Visitor::visitFunc_signature(yupc::YupParser::Func_signatureContext *ctx) 
{
    std::string name = ctx->IDENTIFIER()->getText();        

    this->visit(ctx->type_annot());
    llvm::Type *return_type = yupc::comp_units.back()->type_stack.top();

    std::vector<yupc::FuncParam*> params;
    for (yupc::YupParser::Func_paramContext *p : ctx->func_param()) 
    {
        yupc::FuncParam *fp = std::any_cast<yupc::FuncParam*>(this->visit(p));
        params.push_back(fp);
    }

    std::vector<llvm::Type*> param_types;
    for (auto const *pt : params) 
    {
        param_types.push_back(pt->param_type);
    }

    bool is_pub = ctx->PUBSYM() != nullptr;

    yupc::func_sig_codegen(is_pub, name, return_type, param_types, params);

    return nullptr;
}

std::any yupc::Visitor::visitFunc_param(yupc::YupParser::Func_paramContext *ctx) 
{

    this->visit(ctx->type_annot());
    llvm::Type *resolved_type = yupc::comp_units.back()->type_stack.top();

    std::string name = ctx->IDENTIFIER()->getText();

    yupc::FuncParam *func_param = new yupc::FuncParam{resolved_type, name};
    return func_param;
}

std::any yupc::Visitor::visitFunc_def(yupc::YupParser::Func_defContext *ctx) 
{
    std::string func_name = ctx->func_signature()->IDENTIFIER()->getText();

    this->visit(ctx->func_signature());
    llvm::Function *function = yupc::comp_units.back()->functions[func_name];
    
    if (!function) 
    {
        yupc::log_compiler_err("cannot resolve the signature for function " + func_name, ctx->getText());
        exit(1);
    }

    yupc::func_def_codegen(function);

    bool is_void = function->getReturnType()->isVoidTy();
    if (!is_void) 
    {
        this->visit(ctx->code_block());
        llvm::Value *ret_value = yupc::comp_units.back()->value_stack.top();

        yupc::comp_units.back()->ir_builder->CreateRet(ret_value);

        yupc::comp_units.back()->value_stack.pop();
    } 
    else 
    {
        this->visit(ctx->code_block());
        yupc::comp_units.back()->ir_builder->CreateRetVoid();
    }


    verifyFunction(*function, &llvm::errs());
    yupc::comp_units.back()->symbol_table.pop_back();

    return nullptr;
}

std::any yupc::Visitor::visitFunc_call(yupc::YupParser::Func_callContext *ctx) 
{
    std::string func_name = ctx->IDENTIFIER()->getText();

    auto find = yupc::comp_units.back()->symbol_table.back().find(func_name);
    auto end = yupc::comp_units.back()->symbol_table.back().end();

    if (find != end) 
    {
        yupc::log_compiler_err("cannot call function \"" + func_name + "\" because it doesn't exist in the symbol table", ctx->getText());
        exit(1);
    }

    std::vector<llvm::Value*> args;
    size_t expr_length = ctx->expr().size();
    for (size_t i = 0; i < expr_length; ++i) 
    {
        yupc::YupParser::ExprContext *expr = ctx->expr()[i];

        this->visit(expr);
        llvm::Value *arg_val = yupc::comp_units.back()->value_stack.top();

        args.push_back(arg_val);

        yupc::comp_units.back()->value_stack.pop();
    }

    yupc::func_call_codegen(func_name, args, ctx->getText());
    
    return nullptr;
}
