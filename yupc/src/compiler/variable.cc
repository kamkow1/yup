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
    this->visit(ctx->expr());
    Value* val = valueStack.top();

    Type *type = val->getType();
    AllocaInst *ptr = irBuilder.CreateAlloca(type, 0, name);
    irBuilder.CreateStore(val, ptr, false);
    symbolTable[name] = ptr;

    valueStack.push(ptr);

    bool isConst = (ctx->CONST() != nullptr);
    Variable var{name, isConst};
    variables[name] = var;

    return nullptr;
}

std::any Visitor::visitAssignment(YupParser::AssignmentContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    // push value to value stack
    this->visit(ctx->expr());
    Value *val = valueStack.top();

    Variable var = variables[name];
    bool isConst = var.isConst;

    // check passes if program doesn't exit
    checkValueType(val, name);

    if (isConst)
    {
        logCompilerError("cannot reassign a constant \"" + name + "\"");
        exit(1);
    }

    Type *type = val->getType();
    AllocaInst *stored = symbolTable[name];
    irBuilder.CreateStore(val, stored, false);

    valueStack.pop();
    
    return nullptr;
}

std::any Visitor::visitIdentifierExpr(YupParser::IdentifierExprContext *ctx)
{
    // push value
    std::string name = ctx->IDENTIFIER()->getText();
    AllocaInst *value = symbolTable[name];
    Type *type = value->getAllocatedType();

    LoadInst *load = irBuilder.CreateLoad(type, value, name);
    valueStack.push(load);
    return nullptr;
}