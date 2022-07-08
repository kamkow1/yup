#pragma once
#include "../parser/YupParserBaseVisitor.h"
#include "llvm/IR/IRBuilder.h"

class Visitor : public YupParserBaseVisitor
{
public:
    llvm::Value *codegen();

    std::any visitFunc_def(YupParser::Func_defContext *ctx) override;

    std::any visitFile(YupParser::FileContext *ctx) override;
};