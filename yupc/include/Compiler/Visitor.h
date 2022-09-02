#ifndef YUPC_COMPILER_VISITOR_H_
#define YUPC_COMPILER_VISITOR_H_

#include "Parser/YupParser.h"
#include "Parser/YupParserBaseVisitor.h"

#include <any>

namespace yupc 
{
    class Visitor : public yupc::YupParserBaseVisitor 
    {
    public:
        std::any visitFile(yupc::YupParser::FileContext *ctx)                                           override;
        std::any visitCodeBlock(yupc::YupParser::CodeBlockContext *ctx)                                 override;

        std::any visitFunctionDefinition(yupc::YupParser::FunctionDefinitionContext *ctx)               override;
        std::any visitFunctionParameter(yupc::YupParser::FunctionParameterContext *ctx)                 override;
        std::any visitFunctionSignature(yupc::YupParser::FunctionSignatureContext *ctx)                 override;
        std::any visitFunctionReturn(yupc::YupParser::FunctionReturnContext *ctx)                       override;
        std::any visitFunctionCall(yupc::YupParser::FunctionCallContext *ctx)                           override;

        std::any visitAssignment(yupc::YupParser::AssignmentContext *ctx)                               override;
        std::any visitVariableDeclare(yupc::YupParser::VariableDeclareContext *ctx)                     override;
        std::any visitIdentifierExpression(yupc::YupParser::IdentifierExpressionContext *ctx)           override;

        std::any visitTypeAnnotation(yupc::YupParser::TypeAnnotationContext *ctx)                       override;
        std::any visitTypeDeclaration(yupc::YupParser::TypeDeclarationContext *ctx)                     override;
        std::any visitTypeCastExpression(yupc::YupParser::TypeCastExpressionContext *ctx)               override;
        std::any visitTypeName(yupc::YupParser::TypeNameContext *ctx)                                   override;
        std::any visitTypeExpression(yupc::YupParser::TypeExpressionContext *ctx)                       override;
        std::any visitTypeNameExpression(yupc::YupParser::TypeNameExpressionContext *ctx)               override;

        std::any visitPointerDereference(yupc::YupParser::PointerDereferenceContext *ctx)               override;
        std::any visitAddressOf(yupc::YupParser::AddressOfContext *ctx)                                 override;

        std::any visitArray(yupc::YupParser::ArrayContext *ctx)                                         override;
        std::any visitIndexedAccessExpression(yupc::YupParser::IndexedAccessExpressionContext *ctx)     override;
        std::any visitArrayElementAssignment(yupc::YupParser::ArrayElementAssignmentContext *ctx)       override;

        std::any visitEmphasizedExpression(yupc::YupParser::EmphasizedExpressionContext *ctx)           override;
        std::any visitBinaryOperationExpression(yupc::YupParser::BinaryOperationExpressionContext *ctx) override;

        std::any visitIfStatement(yupc::YupParser::IfStatementContext *ctx)								override;

        std::any visitComparisonExpression(yupc::YupParser::ComparisonExpressionContext *ctx)           override;

        std::any visitOperator(yupc::YupParser::OperatorContext *ctx)                                   override;

        std::any visitImportDeclaration(yupc::YupParser::ImportDeclarationContext *ctx)                 override;

        std::any visitConstant(yupc::YupParser::ConstantContext *ctx)                                   override;
    };
} // namespace yupc

#endif
