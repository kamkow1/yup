#include "compiler/visitor.h"
#include "compiler/type.h"
#include "compiler/function.h"
#include "msg/errors.h"
#include "util.h"

#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Comdat.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"

#include "string.h"

using namespace llvm;
using namespace yupc;
using namespace yupc::msg::errors;


namespace cv = compiler::visitor;
namespace cf = compiler::function;

static std::map<std::string, Function*> function_table;

void cf::func_def_codegen(Function *function)
{
    BasicBlock *block = BasicBlock::Create(cv::context, "entry", function);
    cv::ir_builder.SetInsertPoint(block);

    std::map<std::string, AllocaInst*> map;
    cv::symbol_table.push(map);

    size_t len = function->arg_size();
    for (size_t i = 0; i < len; ++i)
    {
        Argument &arg = *function->getArg(i);

        AllocaInst *alloca = cv::ir_builder.CreateAlloca(
            arg.getType(), 0, arg.getName());

        cv::symbol_table.top()[arg.getName().str()] = alloca;
        cv::ir_builder.CreateStore(&arg, alloca);
    }
}

void cf::func_call_codegen(std::string func_name, size_t expr_length, 
                        std::vector<Value*> args)
{
    Function *fn_callee = cv::module->getFunction(func_name);

    if (fn_callee == nullptr)
    {
        log_compiler_err("tried to call function " 
            + func_name + " but it isn't declared");
        exit(1);
    }

    if (args.size() != fn_callee->arg_size())
    {
        log_compiler_err("found function \"" + func_name
            + "\" but couldn't match given argument" 
            + " list length to the function signature");
        exit(1);
    }


    bool is_void = fn_callee->getFunctionType()
                            ->getReturnType()
                            ->isVoidTy();
    if (is_void)
    {
        CallInst *result = cv::ir_builder.CreateCall(fn_callee, args, "");
        cv::value_stack.push(result);
    }
    else
    {
        std::string call_label = std::string("call") + "_" + func_name;
        CallInst *result = cv::ir_builder.CreateCall(fn_callee, args, call_label);
        cv::value_stack.push(result);
    }
}

void cf::func_sig_codegen(bool is_ext, std::string name, Type *return_type, 
    std::vector<llvm::Type*> param_types, std::vector<compiler::function::FuncParam*> params)
{
    FunctionType *fn_type = FunctionType::get(
            return_type,
            param_types,
            false);

    Function *function = Function::Create(
            fn_type,
            is_ext || name == "main"
                ? GlobalValue::ExternalLinkage 
                : GlobalValue::InternalLinkage,
            name,
            cv::module.get());

    function->setDSOLocal(!is_ext);

    int arg_max = function->arg_size();
    for (int i = 0; i < arg_max; ++i)
    {
        std::string pn = params[i]->param_name;
        function->getArg(i)->setName(pn);
    }

    function_table[name] = function;
}

std::any cv::Visitor::visitFunc_return(parser::YupParser::Func_returnContext *ctx)
{
    std::any value = this->visit(ctx->expr());
    return value;
}

std::any cv::Visitor::visitFunc_signature(parser::YupParser::Func_signatureContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();        
    Type *return_type = std::any_cast<Type*>(
        this->visit(ctx->type_annot()));

    std::vector<cf::FuncParam*> params;
    for (parser::YupParser::Func_paramContext *const p : ctx->func_param())
    {
        cf::FuncParam *fp = std::any_cast<cf::FuncParam*>(this->visit(p));
        params.push_back(fp);
    }

    std::vector<Type*> param_types;
    for (const cf::FuncParam *pt : params)
    {
        param_types.push_back(pt->param_type);
    }

    bool is_EXPORT = ctx->EXPORT() != nullptr;

    cf::func_sig_codegen(is_EXPORT, 
                    name, 
                    return_type, 
                    param_types, 
                    params);

    return nullptr;
}

std::any cv::Visitor::visitFunc_param(parser::YupParser::Func_paramContext *ctx)
{
    Type *resolved_type = std::any_cast<Type*>(
        this->visit(ctx->type_annot()));

    std::string name = ctx->IDENTIFIER()->getText();

    cf::FuncParam *func_param = new cf::FuncParam{resolved_type, name};
    return func_param;
}

std::any cv::Visitor::visitFunc_def(parser::YupParser::Func_defContext *ctx)
{
    std::string func_name = ctx
            ->func_signature()
            ->IDENTIFIER()
            ->getText();

    this->visit(ctx->func_signature());
    Function *function = function_table[func_name];
    
    if (!function)
    {
        log_compiler_err("cannot resolve the signature for function " + func_name);
        exit(1);
    }

    cf::func_def_codegen(function);

    bool is_void = function->getReturnType()->isVoidTy();
    if (!is_void)
    {
        this->visit(ctx->code_block());

        Value *ret_value = value_stack.top();
        cv::ir_builder.CreateRet(ret_value);
        value_stack.pop();
    }
    else
    {
        this->visit(ctx->code_block());
        cv::ir_builder.CreateRetVoid();
    }

    verifyFunction(*function, &outs());
    cv::symbol_table.pop();

    return nullptr;
}

std::any cv::Visitor::visitFunc_call(parser::YupParser::Func_callContext *ctx)
{
    std::string func_name = ctx->IDENTIFIER()->getText();

    if (cv::symbol_table.top().find(func_name) != cv::symbol_table.top().end())
    {
        log_compiler_err("cannot call function \"" + func_name 
            + "\" because it doesn't exist in the symbol table");
        exit(1);
    }

    std::vector<Value*> args;
    size_t expr_length = ctx->expr().size();
    for (size_t i = 0; i < expr_length; ++i)
    {
        parser::YupParser::ExprContext* expr = ctx->expr()[i];
        this->visit(expr);
        Value *arg_val = value_stack.top();
        args.push_back(arg_val);
        cv::value_stack.pop();
    }

    cf::func_call_codegen(func_name, expr_length, args);
    
    return nullptr;
}
