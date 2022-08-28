#include "Compiler/CompilationUnit.h"
#include "Compiler/Logic.h"
#include "Compiler/Type.h"
#include "Compiler/Visitor.h"
#include "Logger.h"

#include "llvm/IR/Value.h"
#include "llvm/IR/InstrTypes.h"

llvm::Value *yupc::EqualCodegen(llvm::Value *lhs, llvm::Value *rhs)
{
    llvm::Value *cmp = yupc::CompilationUnits.back()->IRBuilder->CreateCmp(llvm::CmpInst::ICMP_EQ, lhs, rhs);
    return cmp;
}

llvm::Value *yupc::NotEqualCodegen(llvm::Value *lhs, llvm::Value *rhs)
{
    llvm::Value *cmp = yupc::CompilationUnits.back()->IRBuilder->CreateCmp(llvm::CmpInst::ICMP_NE, lhs, rhs);
    return cmp;
}

std::any yupc::Visitor::visitComparisonExpression(yupc::YupParser::ComparisonExpressionContext *ctx)
{
    this->visit(ctx->expression(0));
    llvm::Value *val1 = yupc::CompilationUnits.back()->ValueStack.top();

    this->visit(ctx->expression(1));
    llvm::Value *val2 = yupc::CompilationUnits.back()->ValueStack.top();

    yupc::CompilationUnits.back()->ValueStack.pop();
    yupc::CompilationUnits.back()->ValueStack.pop();

    if (!yupc::CheckValueType(val1, val2))
    {
        yupc::GlobalLogger.LogCompilerError(ctx->start->getLine(), ctx->start->getCharPositionInLine(), 
                                            "cannot compare values of different types", ctx->getText(), 
                                            yupc::CompilationUnits.back()->SourceFile);
        exit(1);
    }

    llvm::Value *val;

    if (ctx->comparisonOperator()->SymbolEqual() != nullptr)
    {
        val = yupc::EqualCodegen(val1, val2);
    }
    else if (ctx->comparisonOperator()->SymbolNotEqual() != nullptr)
    {
        val = yupc::NotEqualCodegen(val1, val2);
    }

    yupc::CompilationUnits.back()->ValueStack.push(val);
    return nullptr;
}
