#include "compiler/visitor.h"
#include "util.h"
#include "messaging/errors.h"
#include "llvm/Support/TypeName.h"
#include "llvm/Support/Alignment.h"
#include "compiler/type.h"
#include "compiler/variable.h"

using namespace llvm;

struct Variable
{
    std::string name;
    bool        isConst;
};

static std::map<std::string, Variable> variables;

void identExpr_codegen(std::string id)
{
    AllocaInst *value = symbolTable.top()[id];
    Type *type = value->getAllocatedType();
    LoadInst *load = irBuilder.CreateLoad(type, value);
    valueStack.push(load);
}

void assignment_codegen(std::string name, Value *val)
{
    Variable var = variables[name];
    bool isConst = var.isConst;

    if (isConst)
    {
        logCompilerError("cannot reassign a constant \"" + name + "\"");
        exit(1);
    }

    AllocaInst *stored = symbolTable.top()[name];

    checkValueType(val, name);

    irBuilder.CreateStore(val, stored, false);

    valueStack.pop();
}

void varDeclare_codegen(std::string name, Type *resolvedType, 
                        bool isConst, Value *val)
{
    AllocaInst *ptr 
        = irBuilder.CreateAlloca(resolvedType, 0, "");

    if (val != nullptr)
    {
        irBuilder.CreateStore(val, ptr, false);
    }

    symbolTable.top()[name] = ptr;

    valueStack.push(ptr);

    //bool isConst = (ctx->CONST() != nullptr);
    Variable var{name, isConst};
    variables[name] = var;
}

std::any Visitor::visitVar_declare(YupParser::Var_declareContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    Type *resolvedType 
        = std::any_cast<Type*>(this->visit(ctx->type_annot()));
    
    bool isConst = ctx->CONST() != nullptr;

    Value *val = nullptr;
    if (ctx->var_value() != nullptr)
    {
        this->visit(ctx->var_value()->expr());
        val = valueStack.top();
    }

    varDeclare_codegen(name, resolvedType, isConst, val);

    return nullptr;
}

std::any Visitor::visitAssignment(YupParser::AssignmentContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    this->visit(ctx->var_value()->expr());
    Value *val = valueStack.top();

    assignment_codegen(name, val);
    
    return nullptr;
}

std::any Visitor::visitIdentifierExpr(YupParser::IdentifierExprContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    identExpr_codegen(name);
    
    return nullptr;
}