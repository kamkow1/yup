#include "compiler/visitor.h"
#include "compiler/type.h"
#include "compiler/variable.h"
#include "msg/errors.h"
#include "util.h"

#include "llvm/Support/TypeName.h"
#include "llvm/Support/Alignment.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Metadata.h"

#include "cstdlib"

using namespace llvm;
using namespace yupc;
using namespace yupc::msg::errors;

namespace cv = compiler::visitor;
namespace cvar = compiler::variable;
namespace ct = compiler::type;

struct Variable
{
    std::string name;
    bool        is_const;
};

static std::map<std::string, Variable> variables;

void cvar::ident_expr_codegen(std::string id)
{
    if (cv::symbol_table.top().contains(id))
    {
        AllocaInst *value = cv::symbol_table.top()[id];
        Type *type = value->getAllocatedType();
        LoadInst *load = cv::ir_builder.CreateLoad(type, value);

        cv::value_stack.push(load);
    }
    else
    {
        GlobalVariable *gv = cv::global_variables[id];
        Type *type = gv->getValueType();
        LoadInst *load = cv::ir_builder.CreateLoad(type, gv);

        cv::value_stack.push(load);
    }

}

void cvar::assignment_codegen(std::string name, Value *val)
{
    Variable var = variables[name];

    if (var.is_const)
    {
        log_compiler_err("cannot reassign a constant \"" + name + "\"");
        exit(1);
    }

    if (cv::symbol_table.top().count(name))
    {
        AllocaInst *stored = cv::symbol_table.top()[name];
        ct::check_value_type(val, name);

        cv::ir_builder.CreateStore(val, stored, false);

        cv::value_stack.pop();
    }
    else
    {
        GlobalVariable *gv = cv::global_variables[name];
        ct::check_value_type(val, name);

        cv::ir_builder.CreateStore(val, gv, false);

        cv::value_stack.pop();
    }
}

void cvar::var_declare_codegen(std::string name, Type *resolved_type, 
                        bool is_const, bool is_glob, bool is_ext, Value *val)
{
    if (is_glob)
    {
        GlobalValue::LinkageTypes lt = is_ext 
            ? GlobalValue::ExternalLinkage 
            : GlobalValue::InternalLinkage;

        GlobalVariable *gv = new GlobalVariable(*cv::module, 
                                                resolved_type, 
                                                is_const, 
                                                lt, 0);


        gv->setDSOLocal(true);

        gv->setInitializer((Constant*) val);
        cv::global_variables[name] = gv;

        cv::value_stack.push(gv);

        Variable var{name, is_const};
        variables[name] = var;
    }
    else
    {
        AllocaInst *ptr = cv::ir_builder.CreateAlloca(resolved_type, 0, "");

        if (val != nullptr)
        {
            cv::ir_builder.CreateStore(val, ptr, false);
        }

        cv::symbol_table.top()[name] = ptr;

        cv::value_stack.push(ptr);

        Variable var{name, is_const};
        variables[name] = var;
    }
}

std::any cv::Visitor::visitVar_declare(parser::YupParser::Var_declareContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();
    
    bool is_const = ctx->CONST() != nullptr;
    bool is_glob = ctx->GLOBAL() != nullptr;
    bool is_ext = ctx->EXPORT() != nullptr;

    if (is_glob && cv::global_variables.contains(name))
    {
        msg::errors::log_compiler_err("global variable \"" + name 
            + ctx->type_annot()->getText() + "\" already exists");

        exit(1);
    }

    if (!is_glob && cv::symbol_table.top().contains(name))
    {
        msg::errors::log_compiler_err("variable \"" + name 
            + ctx->type_annot()->getText()
            + "\" has already been declared in this scope");
        exit(1);
    }

    Type *resolved_type = std::any_cast<Type*>(
        this->visit(ctx->type_annot()));

    Value *val = nullptr;
    if (ctx->var_value() != nullptr)
    {
        this->visit(ctx->var_value()->expr());
        val = value_stack.top();
    }

    cvar::var_declare_codegen(name, 
                            resolved_type, 
                            is_const, 
                            is_glob, 
                            is_ext, 
                            val);

    return nullptr;
}

std::any cv::Visitor::visitAssignment(parser::YupParser::AssignmentContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    this->visit(ctx->var_value()->expr());
    Value *val = value_stack.top();

    cvar::assignment_codegen(name, val);
    
    return nullptr;
}

std::any cv::Visitor::visitIdentifierExpr(parser::YupParser::IdentifierExprContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    cvar::ident_expr_codegen(name);
    
    return nullptr;
}