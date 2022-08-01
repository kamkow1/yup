#include "compiler/visitor.h"
#include "compiler/type.h"
#include "compiler/function.h"
#include "messaging/errors.h"
#include "util.h"

#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "string.h"

using namespace llvm;
using namespace YupCompiler;

static std::map<std::string, Function*> function_table;

void func_def_codegen(Function *function)
{
    BasicBlock *block = BasicBlock::Create(context, "entry", function);
    ir_builder.SetInsertPoint(block);

    std::map<std::string, AllocaInst*> map;
    symbol_table.push(map);

    size_t len = function->arg_size();
    for (size_t i = 0; i < len; ++i)
    {
        Argument &arg = *function->getArg(i);

        AllocaInst *alloca = ir_builder.CreateAlloca(
            arg.getType(), 0, arg.getName());

        symbol_table.top()[arg.getName().str()] = alloca;
        ir_builder.CreateStore(&arg, alloca);
    }
}

void func_call_codegen(std::string func_name, 
                    size_t expr_length, std::vector<Value*> args)
{
    Function *fn_callee = module->getFunction(func_name);

    if (fn_callee == nullptr)
    {
        log_compiler_err("tried to call function " + func_name + " but it isn't declared");
        exit(1);
    }

    if (args.size() != fn_callee->arg_size())
    {
        log_compiler_err("found function \"" + func_name
            + "\" but couldn't match given argument" 
            + " list length to the function signature");
        exit(1);
    }


    bool is_void = fn_callee->getReturnType()->isVoidTy();
    if (is_void)
    {
        CallInst *result = ir_builder.CreateCall(fn_callee, args, "");
        value_stack.push(result);
    }
    else
    {
        std::string call_label = std::string("call") + "_" + func_name;
        CallInst *result = ir_builder.CreateCall(fn_callee, args, call_label);
        value_stack.push(result);
    }
}

void func_sig_codegen(bool is_external, std::string name, Type *return_type, 
    std::vector<llvm::Type*> param_types, std::vector<FuncParam*> params)
{
    FunctionType *fn_type = FunctionType::get(
            return_type,
            param_types,
            false);

    Function *function = Function::Create(
            fn_type,
            is_external
                ? GlobalValue::ExternalLinkage 
                : GlobalValue::PrivateLinkage,
            name,
            module.get());

    function->setDSOLocal(!is_external);

    int arg_max = function->arg_size();
    for (int i = 0; i < arg_max; ++i)
    {
        std::string pn = params[i]->param_name;
        function->getArg(i)->setName(pn);
    }

    function_table[name] = function;
}

std::any Visitor::visitFunc_return(Parser::YupParser::Func_returnContext *ctx)
{
    std::any value = this->visit(ctx->expr());
    return value;
}

std::any Visitor::visitFunc_signature(Parser::YupParser::Func_signatureContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();        
    Type *return_type = std::any_cast<Type*>(
        this->visit(ctx->type_annot()));

    std::vector<FuncParam*> params;
    for (Parser::YupParser::Func_paramContext *const p : ctx->func_param())
    {
        FuncParam *fp = std::any_cast<FuncParam*>(this->visit(p));
        params.push_back(fp);
    }

    std::vector<Type*> param_types;
    for (const FuncParam *pt : params)
    {
        param_types.push_back(pt->param_type);
    }

    bool is_external = ctx->EXTERNAL() != nullptr;

    func_sig_codegen(is_external, 
                    name, 
                    return_type, 
                    param_types, 
                    params);

    return nullptr;
}

std::any Visitor::visitFunc_param(Parser::YupParser::Func_paramContext *ctx)
{
    Type *resolved_type = std::any_cast<Type*>(
        this->visit(ctx->type_annot()));

    std::string name = ctx->IDENTIFIER()->getText();

    FuncParam *func_param = new FuncParam{resolved_type, name};
    return func_param;
}

std::any Visitor::visitFunc_def(parser::YupParser::Func_defContext *ctx)
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

    func_def_codegen(function);

    bool is_void = function->getReturnType()->isVoidTy();
    if (!is_void)
    {
        this->visit(ctx->code_block());

        Value *ret_value = value_stack.top();
        ir_builder.CreateRet(ret_value);
        value_stack.pop();
    }
    else
    {
        this->visit(ctx->code_block());
        ir_builder.CreateRetVoid();
    }

    verifyFunction(*function, &outs());
    symbol_table.pop();

    return nullptr;
}

std::any Visitor::visitFunc_call(Parser::YupParser::Func_callContext *ctx)
{
    std::string func_name = ctx->IDENTIFIER()->getText();

    if (symbol_table.top().find(func_name) != symbol_table.top().end())
    {
        log_compiler_err("cannot call function \"" + func_name 
            + "\" because it doesn't exist in the symbol table");
        exit(1);
    }

    std::vector<Value*> args;
    size_t expr_length = ctx->expr().size();
    for (size_t i = 0; i < expr_length; ++i)
    {
        Parser::YupParser::ExprContext* expr = ctx->expr()[i];
        this->visit(expr);
        Value *arg_val = value_stack.top();
        args.push_back(arg_val);
        value_stack.pop();
    }

    func_call_codegen(func_name, expr_length, args);
    
    return nullptr;
}
