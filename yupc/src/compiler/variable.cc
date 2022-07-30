#include "visitor.h"
#include "util.h"
#include "messaging/errors.h"
#include "llvm/Support/TypeName.h"
#include "llvm/Support/Alignment.h"
#include "compiler/type.h"

using namespace llvm;

struct Variable
{
    std::string name;
    bool        isConst;
};

static std::map<std::string, Variable> variables;

std::any Visitor::visitVar_declare(YupParser::Var_declareContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();
    // TypeAnnotation type = 
    //     std::any_cast<TypeAnnotation>(this->visit(ctx->type_annot()));

    Type *resolvedType 
        = std::any_cast<Type*>(this->visit(ctx->type_annot()));
    AllocaInst *ptr 
        = irBuilder.CreateAlloca(resolvedType, 0, "");

    if (ctx->var_value() != nullptr)
    {
        this->visit(ctx->var_value()->expr());
        Value* val = valueStack.top();
        irBuilder.CreateStore(val, ptr, false);
    }

    symbolTable.top()[name] = ptr;

    valueStack.push(ptr);

    bool isConst = (ctx->CONST() != nullptr);
    Variable var{name, isConst};
    variables[name] = var;

    return nullptr;
}

std::any Visitor::visitAssignment(YupParser::AssignmentContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    Variable var = variables[name];
    bool isConst = var.isConst;

    if (isConst)
    {
        logCompilerError("cannot reassign a constant \"" + name + "\"");
        exit(1);
    }

    AllocaInst *stored = symbolTable.top()[name];

    this->visit(ctx->var_value()->expr());
    Value *val = valueStack.top();
    checkValueType(val, name);

    irBuilder.CreateStore(val, stored, false);

    valueStack.pop();
    
    return nullptr;
}

std::any Visitor::visitIdentifierExpr(YupParser::IdentifierExprContext *ctx)
{
    // push value
    std::string name = ctx->IDENTIFIER()->getText();
    AllocaInst *value = symbolTable.top()[name];
    Type *type = value->getAllocatedType();
    LoadInst *load = irBuilder.CreateLoad(type, value, name);
    valueStack.push(load);
    return nullptr;
}