#include <visitor.h>
#include <util.h>
#include <messaging/errors.h>
#include <llvm/Support/TypeName.h>

using namespace llvm;

std::any Visitor::visitAssignment(YupParser::AssignmentContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    // push value to value stack
    this->visit(ctx->expr());
    Value* val = valueStack.top();

    // assert type
    if (symbolTable.find(name) != symbolTable.end())
    {
        bool isConst = ctx->CONST() == nullptr;
        if (isConst)
        {
            std::string errorMessage 
                = "cannot reassign a constant \"" + name + "\"";
            logCompilerError(errorMessage);
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

        if (exprType != ogType)
        {
            std::string errorMessage 
                = "mismatch of types \"" + ogType 
                + "\" and \"" + exprType + "\"";

            logCompilerError(errorMessage);
            exit(1);
        }
    }

    Type *type = val->getType();
    AllocaInst *ptr = irBuilder.CreateAlloca(type, 0, name);
    Type *allocType = ptr->getAllocatedType();
    irBuilder.CreateStore(val, ptr);

    std::string tn;
    raw_string_ostream rso(tn);
    ptr->getType()->print(rso);
    std::cout << rso.str() << "\n";

    // save
    symbolTable[name] = ptr;

    valueStack.pop();

    return true;
}

std::any Visitor::visitIdentifierExpr(YupParser::IdentifierExprContext *ctx)
{
    // push value
    std::string name = ctx->IDENTIFIER()->getText();
    llvm::Value* val = symbolTable[name];
    valueStack.push(val);

    return irBuilder.CreateLoad(val->getType(), val, name);
}

std::any Visitor::visitRef_expr(YupParser::Ref_exprContext *ctx)
{
    this->visit(ctx->expr());
    llvm::Value* value = valueStack.top();
    valueStack.pop();
    valueStack.push(value);
    return nullptr;
}

std::any Visitor::visitDeref_expr(YupParser::Deref_exprContext *ctx)
{
    this->visit(ctx->expr());

    llvm::Value* var = valueStack.top();
    llvm::Constant* pointerAsInt = llvm::ConstantInt::get(
        context, llvm::APInt(64, reinterpret_cast<uint64_t>(&var))); 

    llvm::Value* pointer = irBuilder.CreateIntToPtr(
        pointerAsInt, llvm::Type::getInt64Ty(context));

    llvm::Value* value = irBuilder.CreateLoad(
        llvm::Type::getInt64Ty(context), pointer);

    valueStack.push(value);

    valueStack.pop();
    return nullptr;
}