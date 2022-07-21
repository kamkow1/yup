#include <visitor.h>
#include <util.h>
#include <messaging/errors.h>
#include <llvm/Support/TypeName.h>
#include <llvm/Support/Alignment.h>

using namespace llvm;

struct Variable
{
    std::string name;
    bool        isConst;
};

static std::map<std::string, Variable> variables;

std::any Visitor::visitAssignment(YupParser::AssignmentContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    // push value to value stack
    this->visit(ctx->expr());
    Value* val = valueStack.top();

    // assert type
    // let ... -> declaration
    // no let -> assume variable exists
    bool isDeclaration = (ctx->LET() != nullptr);
    if (!isDeclaration)
    {
        Variable var = variables[name];
        bool isConst = var.isConst;
        if (isConst)
        {
            logCompilerError("cannot reassign a constant \"" + name + "\"");
            exit(1);
        }

        std::string exprType;
        raw_string_ostream rso(exprType);
        val->getType()->print(rso);
        exprType = getReadableTypeName(rso.str());

        Value* ogVal = symbolTable[name];
        std::string ogType;
        raw_string_ostream ogRso(ogType);
        ogVal->getType()->print(ogRso);
        ogType = getReadableTypeName(ogRso.str());

        if (ogType[ogType.length() - 1] == '*')
        {
            int pos = ogType.find('*');
            if (pos != std::string::npos)
            {
                ogType.erase(pos);
            }
        }

        if (exprType != ogType)
        {
            logCompilerError("mismatch of types \"" + ogType 
                + "\" and \"" + exprType + "\"");
            exit(1);
        }
    }

    Type *type = val->getType();
    AllocaInst *ptr = irBuilder.CreateAlloca(type, 0, name);
    irBuilder.CreateStore(val, ptr, false);
    symbolTable[name] = ptr;

    valueStack.pop();

    bool isConst = ctx->CONST() != nullptr;

    // init
    Variable var = {
        name,
        isConst
    };
    variables[name] = var;

    return nullptr;
}

std::any Visitor::visitIdentifierExpr(YupParser::IdentifierExprContext *ctx)
{
    // push value
    std::string name = ctx->IDENTIFIER()->getText();
    AllocaInst *val = symbolTable[name];
    valueStack.push(val);

    return nullptr;
}