#pragma once
#include "parser/YupParser.h"
#include "parser/YupParserBaseVisitor.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalVariable.h"

#include "vector"
#include "iostream"
#include "map"
#include "sstream"
#include "iostream"
#include "stack"

namespace yupc::compiler::visitor {
    namespace parser = yupc::parser;

    class Visitor : public parser::YupParserBaseVisitor {
    public:
        std::any visitFunc_def(parser::YupParser::Func_defContext *ctx) override;

        std::any visitFile(parser::YupParser::FileContext *ctx) override;

        std::any visitConstant(parser::YupParser::ConstantContext *ctx) override;

        std::any visitType_annot(parser::YupParser::Type_annotContext *ctx) override;

        std::any visitFunc_param(parser::YupParser::Func_paramContext *ctx) override;

        std::any visitFunc_signature(parser::YupParser::Func_signatureContext *ctx) override;

        std::any visitFunc_return(parser::YupParser::Func_returnContext *ctx) override;

        std::any visitFunc_call(parser::YupParser::Func_callContext *ctx) override;

        std::any visitCode_block(parser::YupParser::Code_blockContext *ctx) override;

        std::any visitAssignment(parser::YupParser::AssignmentContext *ctx) override;

        std::any visitIdentifierExpr(parser::YupParser::IdentifierExprContext *ctx) override;

        std::any visitArray(parser::YupParser::ArrayContext *ctx) override;

        std::any visitVar_declare(parser::YupParser::Var_declareContext *ctx) override;

        std::any visitAddr_of(parser::YupParser::Addr_ofContext *ctx) override;

        std::any visitIndexedAccessExpr(parser::YupParser::IndexedAccessExprContext *ctx) override;

        std::any visitArr_elem_assignment(parser::YupParser::Arr_elem_assignmentContext *ctx) override;

        std::any visitMathOperExpr(parser::YupParser::MathOperExprContext *ctx) override;

        std::any visitEmphExpr(parser::YupParser::EmphExprContext *ctx) override;

        std::any visitArray_init(parser::YupParser::Array_initContext *ctx) override;

        std::any visitImport_decl(parser::YupParser::Import_declContext *ctx) override;

        std::any visitModule_decl(parser::YupParser::Module_declContext *ctx) override;

        std::any visitType_decl(parser::YupParser::Type_declContext *ctx) override;
    };
}
