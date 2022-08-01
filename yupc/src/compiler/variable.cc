#include "compiler/visitor.h"
#include "compiler/type.h"
#include "compiler/variable.h"
#include "messaging/errors.h"
#include "util.h"

#include "llvm/Support/TypeName.h"
#include "llvm/Support/Alignment.h"

using namespace llvm;
using namespace YupCompiler;

struct Variable
{
    std::string name;
    bool        is_const;
};

static std::map<std::string, Variable> variables;

void ident_expr_codegen(std::string id)
{
    AllocaInst *value = symbol_table.top()[id];
    Type *type = value->getAllocatedType();
    LoadInst *load = ir_builder.CreateLoad(type, value);
    value_stack.push(load);
}

void assignment_codegen(std::string name, Value *val)
{
    Variable var = variables[name];

    if (var.is_const)
    {
        log_compiler_err("cannot reassign a constant \"" + name + "\"");
        exit(1);
    }

    AllocaInst *stored = symbol_table.top()[name];

    check_value_type(val, name);

    ir_builder.CreateStore(val, stored, false);

    value_stack.pop();
}

void var_declare_codegen(std::string name, Type *resolved_type, 
                        bool is_const, Value *val)
{
    AllocaInst *ptr = ir_builder.CreateAlloca(resolved_type, 0, "");

    if (val != nullptr)
    {
        ir_builder.CreateStore(val, ptr, false);
    }

    symbol_table.top()[name] = ptr;

    value_stack.push(ptr);

    Variable var{name, is_const};
    variables[name] = var;
}

std::any Visitor::visitVar_declare(Parser::YupParser::Var_declareContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    Type *resolved_type = std::any_cast<Type*>(
        this->visit(ctx->type_annot()));
    
    bool is_const = ctx->CONST() != nullptr;

    Value *val = nullptr;
    if (ctx->var_value() != nullptr)
    {
        this->visit(ctx->var_value()->expr());
        val = value_stack.top();
    }

    var_declare_codegen(name, resolved_type, is_const, val);

    return nullptr;
}

std::any Visitor::visitAssignment(Parser::YupParser::AssignmentContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    this->visit(ctx->var_value()->expr());
    Value *val = value_stack.top();

    assignment_codegen(name, val);
    
    return nullptr;
}

std::any Visitor::visitIdentifierExpr(Parser::YupParser::IdentifierExprContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    ident_expr_codegen(name);
    
    return nullptr;
}