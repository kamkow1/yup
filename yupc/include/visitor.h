#pragma once
#include <../src/parser/YupParserBaseVisitor.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <vector>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <iostream>
#include <llvm/Support/FileSystem.h>

class Visitor : public YupParserBaseVisitor
{
public:
    std::any visitFunc_def(YupParser::Func_defContext *ctx) override;
    std::any visitFile(YupParser::FileContext *ctx) override;
    std::any visitConstant(YupParser::ConstantContext *ctx) override;
    std::any visitType_annot(YupParser::Type_annotContext *ctx) override;
    std::any visitFunc_param(YupParser::Func_paramContext *ctx) override;
    std::any visitFunc_signature(YupParser::Func_signatureContext *ctx) override;
    std::any visitFunc_return(YupParser::Func_returnContext *ctx) override;
    std::any visitCompiler_flag(YupParser::Compiler_flagContext *ctx) override;
};

static std::string moduleName;

static llvm::LLVMContext codegenCtx;
static llvm::IRBuilder<> irBuilder(codegenCtx);
static std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>(moduleName, codegenCtx);
static std::map<std::string, llvm::Value*> symbolTable;