#pragma once

#include <parser/YupParser.h>
#include <parser/YupParserBaseVisitor.h>

#include <any>

namespace yupc 
{
    class Visitor : public yupc::YupParserBaseVisitor 
    {
    public:
        std::any visitFunc_def(yupc::YupParser::Func_defContext *ctx) override;

        std::any visitFile(yupc::YupParser::FileContext *ctx) override;

        std::any visitConstant(yupc::YupParser::ConstantContext *ctx) override;

        std::any visitType_annot(yupc::YupParser::Type_annotContext *ctx) override;

        std::any visitFunc_param(yupc::YupParser::Func_paramContext *ctx) override;

        std::any visitFunc_signature(yupc::YupParser::Func_signatureContext *ctx) override;

        std::any visitFunc_return(yupc::YupParser::Func_returnContext *ctx) override;

        std::any visitFunc_call(yupc::YupParser::Func_callContext *ctx) override;

        std::any visitCode_block(yupc::YupParser::Code_blockContext *ctx) override;

        std::any visitAssignment(yupc::YupParser::AssignmentContext *ctx) override;

        std::any visitIdentifierExpr(yupc::YupParser::IdentifierExprContext *ctx) override;

        std::any visitArray(yupc::YupParser::ArrayContext *ctx) override;

        std::any visitVar_declare(yupc::YupParser::Var_declareContext *ctx) override;

        std::any visitAddr_of(yupc::YupParser::Addr_ofContext *ctx) override;

        std::any visitIndexedAccessExpr(yupc::YupParser::IndexedAccessExprContext *ctx) override;

        std::any visitArr_elem_assignment(yupc::YupParser::Arr_elem_assignmentContext *ctx) override;

        std::any visitMathOperExpr(yupc::YupParser::MathOperExprContext *ctx) override;

        std::any visitEmphExpr(yupc::YupParser::EmphExprContext *ctx) override;

        std::any visitImport_decl(yupc::YupParser::Import_declContext *ctx) override;

        std::any visitType_decl(yupc::YupParser::Type_declContext *ctx) override;

        std::any visitPtr_dereference(yupc::YupParser::Ptr_dereferenceContext *ctx) override;

        std::any visitOperator(yupc::YupParser::OperatorContext *ctx) override;

        std::any visitTypeCastExpr(yupc::YupParser::TypeCastExprContext *ctx) override;

        std::any visitType_name(yupc::YupParser::Type_nameContext *ctx) override;

        std::any visitType_name_expr(yupc::YupParser::Type_name_exprContext *ctx) override;

        std::any visitTypeNameExpr(yupc::YupParser::TypeNameExprContext *ctx) override;
    };
}
