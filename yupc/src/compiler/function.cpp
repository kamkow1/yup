#include <compiler/visitor.h>
#include <compiler/type.h>
#include <compiler/function.h>
#include <compiler/compilation_unit.h>
#include <llvm/IR/Attributes.h>
#include <msg/errors.h>
#include <util.h>

#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Comdat.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>

#include <string>

using namespace llvm;
using namespace yupc;
using namespace yupc::msg::errors;


namespace cv = compiler::visitor;
namespace cf = compiler::function;
namespace com_un = compiler::compilation_unit;

//static std::map<std::string, Function*> function_table;

void cf::func_def_codegen(Function *function) {
    auto *block = BasicBlock::Create(com_un::comp_units.back()->module->getContext(), "entry", function);

    com_un::comp_units.back()->ir_builder->SetInsertPoint(block);

    std::map<std::string, AllocaInst*> map;

    com_un::comp_units.back()->symbol_table.push_back(map);

    auto len = function->arg_size();
    for (auto i = 0; i < len; ++i) {
        auto &arg = *function->getArg(i);

        auto *alloca = com_un::comp_units.back()->ir_builder->CreateAlloca(arg.getType(), 0, arg.getName());

        com_un::comp_units.back()->symbol_table.back()[arg.getName().str()] = alloca;

        com_un::comp_units.back()->ir_builder->CreateStore(&arg, alloca);
    }
}

void cf::func_call_codegen(std::string func_name, size_t expr_length, 
                        std::vector<Value*> args, std::string text) {
    Function *function = com_un::comp_units.back()->module->getFunction(func_name);

    if (function == nullptr) {
        log_compiler_err("tried to call function " + func_name + " but it isn't declared", text);
        exit(1);
    }

    auto is_void = function->getFunctionType()->getReturnType()->isVoidTy();
    if (is_void) {
        auto *result = com_un::comp_units.back()->ir_builder->CreateCall(function, args, "");

        for (auto i = 0; i < function->arg_size(); i++) {
            result->addParamAttr(i, Attribute::NoUndef);
        }

        com_un::comp_units.back()->value_stack.push(result);

    } else {
        auto call_label = std::string("call") + "_" + func_name;

        auto *result = com_un::comp_units.back()->ir_builder->CreateCall(function, args, call_label);

        for (auto i = 0; i < function->arg_size(); i++) {
            result->addParamAttr(i, Attribute::NoUndef);
        }

        com_un::comp_units.back()->value_stack.push(result);
    }
}

void cf::func_sig_codegen(bool is_ext, std::string name, Type *return_type, 
                        std::vector<llvm::Type*> param_types, 
                        std::vector<FuncParam*> params, std::string text) {

    auto *fn_type = FunctionType::get(return_type, param_types, false);

    auto linkage_type = is_ext || name == "main" ? GlobalValue::ExternalLinkage : GlobalValue::InternalLinkage;

    auto *function = Function::Create(fn_type, linkage_type, name, com_un::comp_units.back()->module);

    for (auto i = 0; i < function->arg_size(); i++) {
        function->addParamAttr(i, Attribute::NoUndef);
    }

    function->setDSOLocal(!is_ext);

    auto arg_max = function->arg_size();
    for (auto i = 0; i < arg_max; ++i) {
        
        auto pn = params[i]->param_name;
        function->getArg(i)->setName(pn);
    }

    com_un::comp_units.back()->functions[name] = function;
}

std::any cv::Visitor::visitFunc_return(parser::YupParser::Func_returnContext *ctx) {
    auto value = this->visit(ctx->expr());
    return value;
}

std::any cv::Visitor::visitFunc_signature(parser::YupParser::Func_signatureContext *ctx) {
    auto name = ctx->IDENTIFIER()->getText();        

    this->visit(ctx->type_annot());
    auto *return_type = com_un::comp_units.back()->type_stack.top();
    //auto *return_type = std::any_cast<Type*>(this->visit(ctx->type_annot()));

    std::vector<cf::FuncParam*> params;
    for (auto *const p : ctx->func_param()) {
        auto *fp = std::any_cast<cf::FuncParam*>(this->visit(p));
        params.push_back(fp);
    }

    std::vector<Type*> param_types;
    for (auto const *pt : params) {
        param_types.push_back(pt->param_type);
    }

    auto is_pub = ctx->PUBSYM() != nullptr;

    cf::func_sig_codegen(is_pub, name, return_type, param_types, params, ctx->getText());

    return nullptr;
}

std::any cv::Visitor::visitFunc_param(parser::YupParser::Func_paramContext *ctx) {

   //auto *resolved_type = std::any_cast<Type*>(this->visit(ctx->type_annot()));
    this->visit(ctx->type_annot());
    auto *resolved_type = com_un::comp_units.back()->type_stack.top();

    auto name = ctx->IDENTIFIER()->getText();

    auto *func_param = new cf::FuncParam{resolved_type, name};
    return func_param;
}

std::any cv::Visitor::visitFunc_def(parser::YupParser::Func_defContext *ctx) {
    auto func_name = ctx->func_signature()->IDENTIFIER()->getText();

    this->visit(ctx->func_signature());
    auto *function = com_un::comp_units.back()->functions[func_name];
    
    if (!function) {
        log_compiler_err("cannot resolve the signature for function " + func_name, ctx->getText());
        exit(1);
    }

    cf::func_def_codegen(function);

    auto is_void = function->getReturnType()->isVoidTy();
    if (!is_void) {
        this->visit(ctx->code_block());

        auto *ret_value = com_un::comp_units.back()->value_stack.top();

        com_un::comp_units.back()->ir_builder->CreateRet(ret_value);

        com_un::comp_units.back()->value_stack.pop();
    } else {
        this->visit(ctx->code_block());
        com_un::comp_units.back()->ir_builder->CreateRetVoid();
    }

    verifyFunction(*function, &outs());
    com_un::comp_units.back()->symbol_table.pop_back();

    return nullptr;
}

std::any cv::Visitor::visitFunc_call(parser::YupParser::Func_callContext *ctx) {
    auto func_name = ctx->IDENTIFIER()->getText();

    auto find = com_un::comp_units.back()->symbol_table.back().find(func_name);
    auto end = com_un::comp_units.back()->symbol_table.back().end();

    if (find != end) {
        log_compiler_err("cannot call function \"" + func_name + "\" because it doesn't exist in the symbol table", ctx->getText());
        exit(1);
    }

    std::vector<Value*> args;
    auto expr_length = ctx->expr().size();
    for (auto i = 0; i < expr_length; ++i) {
        auto *expr = ctx->expr()[i];

        this->visit(expr);
        auto *arg_val = com_un::comp_units.back()->value_stack.top();

        args.push_back(arg_val);

        com_un::comp_units.back()->value_stack.pop();
    }

    cf::func_call_codegen(func_name, expr_length, args, ctx->getText());
    
    return nullptr;
}
