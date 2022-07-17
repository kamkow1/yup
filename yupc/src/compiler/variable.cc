#include <visitor.h>
#include <util.h>
#include <messaging/errors.h>
#include <llvm/Support/TypeName.h>

std::any Visitor::visitAssignment(YupParser::AssignmentContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    // push value to value stack
    this->visit(ctx->expr());
    llvm::Value* val = valueStack.top();

    std::cout << ctx->getText() << "\n";

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
        llvm::raw_string_ostream rso(exprType);
        val->getType()->print(rso);
        exprType = getReadableTypeName(rso.str());

        llvm::Value* ogVal = symbolTable[name];
        std::string ogType;
        llvm::raw_string_ostream ogRso(ogType);
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

    if (ctx->ASTERISK() != nullptr) // pointer type
    {
        std::cout << "pointer type\n";
        llvm::Constant* addr = llvm::ConstantInt::get(
            llvm::Type::getInt64Ty(codegenCtx), (int64_t) &val);
        llvm::Value* ptr = llvm::ConstantExpr::getIntToPtr(
            addr, llvm::PointerType::getUnqual(llvm::Type::getInt64Ty(codegenCtx)));
        symbolTable[name] = ptr;
    }
    else
    {
        symbolTable[name] = val;
    }

    valueStack.pop();

    return true;
}

std::any Visitor::visitIdentifierExpr(YupParser::IdentifierExprContext *ctx)
{
    // push value
    std::string name = ctx->IDENTIFIER()->getText();
    llvm::Value* val = symbolTable[name];
    valueStack.push(val);

    return nullptr;
}

std::any Visitor::visitRef_expr(YupParser::Ref_exprContext *ctx)
{
    this->visit(ctx->expr());
    llvm::Value* value = valueStack.top();
    valueStack.pop();
    valueStack.push(value);
    return nullptr;
}