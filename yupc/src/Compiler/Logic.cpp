#include "Compiler/CompilationUnit.h"
#include "Compiler/Logic.h"
#include "Compiler/Type.h"
#include "Compiler/Visitor.h"
#include "Logger.h"

#include "llvm/IR/Value.h"
#include "llvm/IR/InstrTypes.h"

llvm::Value *yupc::EqualCodegen(llvm::Value *lhs, llvm::Value *rhs, bool isSigned)
{
    return yupc::CompilationUnits.back()->IRBuilder->CreateCmp(llvm::CmpInst::ICMP_EQ, lhs, rhs);
}

llvm::Value *yupc::NotEqualCodegen(llvm::Value *lhs, llvm::Value *rhs, bool isSigned)
{
    return yupc::CompilationUnits.back()->IRBuilder->CreateCmp(llvm::CmpInst::ICMP_NE, lhs, rhs);
}

llvm::Value *yupc::MoreThanCodegen(llvm::Value *lhs, llvm::Value *rhs, bool isSigned)
{
    return yupc::CompilationUnits.back()->IRBuilder->CreateCmp(llvm::CmpInst::ICMP_UGT, lhs, rhs);
}

llvm::Value *yupc::LessThanCodegen(llvm::Value *lhs, llvm::Value *rhs, bool isSigned)
{
    return yupc::CompilationUnits.back()->IRBuilder->CreateCmp(llvm::CmpInst::ICMP_ULT, lhs, rhs);
}

std::any yupc::Visitor::visitComparisonExpression(yupc::YupParser::ComparisonExpressionContext *ctx)
{
    this->visit(ctx->expression(0));
    llvm::Value *val1 = yupc::CompilationUnits.back()->ValueStack.top();

    this->visit(ctx->expression(1));
    llvm::Value *val2 = yupc::CompilationUnits.back()->ValueStack.top();

    yupc::CompilationUnits.back()->ValueStack.pop();
    yupc::CompilationUnits.back()->ValueStack.pop();

    if (yupc::CheckValueType(val1, val2))
    {
        yupc::GlobalLogger.LogCompilerError(ctx->start->getLine(), ctx->start->getCharPositionInLine(), 
                                            "cannot compare values of different types", ctx->getText(), 
                                            yupc::CompilationUnits.back()->SourceFile);
        exit(1);
    }

    llvm::Value *val;

    if (ctx->comparisonOperator()->compareEqual())
    {
        bool isSigned = ctx->comparisonOperator()->compareEqual()->SymbolEqual() != nullptr;
        val = yupc::EqualCodegen(val1, val2, isSigned);
    }
    else if (ctx->comparisonOperator()->compareNotEqual())
    {
        bool isSigned = ctx->comparisonOperator()->compareNotEqual()->SymbolNotEqual() != nullptr;
        val = yupc::NotEqualCodegen(val1, val2, isSigned);
    }
    else if (ctx->comparisonOperator()->compareMoreThan())
    {
        bool isSigned = ctx->comparisonOperator()->compareMoreThan()->SymbolMoreThan() != nullptr;
        val = yupc::MoreThanCodegen(val1, val2, isSigned);
    }
    else if (ctx->comparisonOperator()->compareLessThan())
    {
        bool isSigned = ctx->comparisonOperator()->compareLessThan()->SymbolLessThan() != nullptr;
        val = yupc::LessThanCodegen(val1, val2, isSigned);
    }

    /*if (ctx->comparisonOperator()->SymbolEqual())
    {
        val = yupc::EqualCodegen(val1, val2, true);
    }
    else if (ctx->comparisonOperator()->SymbolNotEqual())
    {
        val = yupc::NotEqualCodegen(val1, val2, true);
    }
    else if (ctx->comparisonOperator()->SymbolMoreThan())
    {
        val = yupc::MoreThanCodegen(val1, val2, true);
    }
    else if (ctx->comparisonOperator()->SymbolLessThan())
    {
        val = yupc::LessThanCodegen(val1, val2, true);
    }
    else if (ctx->comparisonOperator()->SymbolUnsignedMoreThan())
    {
        val = yupc::MoreThanCodegen(val1, val2, false);
    }
    else if (ctx->comparisonOperator()->SymbolUnsignedLessThan())
    {
        val = yupc::LessThanCodegen(val1, val2, false);
    }*/

    yupc::CompilationUnits.back()->ValueStack.push(val);
    return nullptr;
}
