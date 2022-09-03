package ast

import (
	"fmt"
	"os"
	"reflect"

	"github.com/antlr/antlr4/runtime/Go/antlr"
	"github.com/kamkow1/yup/yupcgo/parser"
)

type AstVisitor struct {
	*parser.BaseYupParserVisitor
}

func NewAstVisitor() *AstVisitor {
	return &AstVisitor{}
}

func (v *AstVisitor) Visit(tree antlr.ParseTree) any {
	switch ctx := tree.(type) {
	case *parser.FileContext:
		return v.VisitFile(ctx)
	case *parser.CodeBlockContext:
		return v.VisitCodeBlock(ctx)
	case *parser.EmphasizedExpressionContext:
		return v.VisitEmphasizedExpression(ctx)
	case *parser.StatementContext:
		return v.VisitStatement(ctx)

	// Type
	case *parser.TypeDeclarationContext:
		return v.VisitTypeDeclaration(ctx)
	case *parser.TypeDefinitionContext:
		return v.VisitTypeDefinition(ctx)
	case *parser.TypeAliasContext:
		return v.VisitTypeAlias(ctx)
	case *parser.TypeAnnotationContext:
		return v.VisitTypeAnnotation(ctx)
	case *parser.TypeNameContext:
		return v.VisitTypeName(ctx)
	case *parser.TypeExtensionContext:
		return v.VisitTypeExtension(ctx)
	case *parser.ArrayTypeExtensionContext:
		return v.VisitArrayTypeExtension(ctx)
	case *parser.TypeNameExpressionContext:
		return v.VisitTypeNameExpression(ctx)
	case *parser.TypeExpressionContext:
		return v.VisitTypeExpression(ctx)

	// Array
	case *parser.ArrayElementAssignmentContext:
		return v.VisitArrayElementAssignment(ctx)
	case *parser.ArrayIndexContext:
		return v.VisitArrayIndex(ctx)
	case *parser.ArrayContext:
		return v.VisitArray(ctx)
	case *parser.ArrayExpressionContext:
		return v.VisitArrayExpression(ctx)
	case *parser.IndexedAccessExpressionContext:
		return v.VisitIndexedAccessExpression(ctx)

	// Variable
	case *parser.AssignmentContext:
		return v.VisitAssignment(ctx)
	case *parser.VariableDeclareContext:
		return v.VisitVariableDeclare(ctx)
	case *parser.DeclarationTypeContext:
		return v.VisitDeclarationType(ctx)
	case *parser.VariableValueContext:
		return v.VisitVariableValue(ctx)
	case *parser.IdentifierExpressionContext:
		return v.VisitIdentifierExpression(ctx)

	// Function
	case *parser.FunctionDefinitionContext:
		return v.VisitFunctionDefinition(ctx)
	case *parser.FunctionSignatureContext:
		return v.VisitFunctionSignature(ctx)
	case *parser.FunctionParameterListContext:
		return v.VisitFunctionParameterList(ctx)
	case *parser.FunctionParameterContext:
		return v.VisitFunctionParameter(ctx)
	case *parser.FunctionReturnContext:
		return v.VisitFunctionReturn(ctx)
	case *parser.FunctionCallContext:
		return v.VisitFunctionCall(ctx)
	case *parser.FunctionCallArgListContext:
		return v.VisitFunctionCallArgList(ctx)
	case *parser.FunctionCallExpressionContext:
		return v.VisitFunctionCallExpression(ctx)

	// Memory
	case *parser.AddressOfContext:
		return v.VisitAddressOf(ctx)
	case *parser.PointerDereferenceContext:
		return v.VisitPointerDereference(ctx)
	case *parser.AddressOfExpressionContext:
		return v.VisitAddressOfExpression(ctx)
	case *parser.PointerDereferenceExpressionContext:
		return v.VisitPointerDereferenceExpression(ctx)

	// Constant
	case *parser.ConstantContext:
		return v.VisitConstant(ctx)
	case *parser.ConstantExpressionContext:
		return v.VisitConstantExpression(ctx)

	// Binary Operator
	case *parser.BinaryOperatorContext:
		return v.VisitBinaryOperator(ctx)
	case *parser.BinaryOperationExpressionContext:
		return v.VisitBinaryOperationExpression(ctx)

	// Operator
	case *parser.OperatorContext:
		return v.VisitOperator(ctx)
	case *parser.OperatorExpressionContext:
		return v.VisitOperatorExpression(ctx)

	// Comparison Operator
	case *parser.ComparisonOperatorContext:
		return v.VisitComparisonOperator(ctx)
	case *parser.ComparisonExpressionContext:
		return v.VisitComparisonExpression(ctx)

	// Import
	case *parser.ImportDeclarationContext:
		return v.VisitImportDeclaration(ctx)

	default:
		fmt.Fprintf(os.Stderr, "ERROR: stepped into an unknown context: %s (type of: %s)\n",
			ctx.GetText(), reflect.TypeOf(ctx).String())
		os.Exit(1)
	}

	return nil
}

func (v *AstVisitor) VisitFile(ctx *parser.FileContext) any {
	for _, st := range ctx.AllStatement() {
		fmt.Printf("%s\n", ctx.GetText())
		v.Visit(st)
	}

	return nil
}

func (v *AstVisitor) VisitStatement(ctx *parser.StatementContext) any {
	return v.Visit(ctx.GetChild(0).(antlr.ParseTree))
}
