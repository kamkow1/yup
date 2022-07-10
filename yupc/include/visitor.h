#pragma once
#include <../src/parser/YupParserBaseVisitor.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Value.h>
#include <vector>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <iostream>
#include <stack>

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
    std::any visitFunc_call(YupParser::Func_callContext *ctx) override;
};

extern std::string moduleName;

extern llvm::LLVMContext codegenCtx;
extern llvm::IRBuilder<> irBuilder;
extern std::unique_ptr<llvm::Module> module;
extern std::map<std::string, llvm::Value*> symbolTable;

extern std::stack<llvm::Value*> valueStack;
