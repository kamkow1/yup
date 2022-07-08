#pragma once
#include "../src/parser/YupParserBaseVisitor.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <vector>
#include <iostream>
#include <map>
#include <sstream>

class Visitor : public YupParserBaseVisitor
{
public:
    std::any visitFunc_def(YupParser::Func_defContext *ctx) override;

    std::any visitFile(YupParser::FileContext *ctx) override;

    std::any visitConstant(YupParser::ConstantContext *ctx) override;
};

static llvm::LLVMContext codegenCtx;
static llvm::IRBuilder<> irBuilder(codegenCtx);
static std::unique_ptr<llvm::Module> module;
static std::map<std::string, llvm::Value*> symbolTable;

// logger
inline llvm::Value *logError(const char* str)
{
    std::stringstream message;
    message << str << "\n";

    fprintf(stderr, "%s", message.str().c_str());
    return nullptr;
}