package compiler

import (
	"log"

	"github.com/antlr/antlr4/runtime/Go/antlr"
	"github.com/kamkow1/yup/yupc/parser"
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
	case *parser.LiteralConstantIntExpressionContext:
		return v.VisitLiteralConstantIntExpression(ctx)
	case *parser.LiteralConstantIntContext:
		return v.VisitLiteralConstantInt(ctx)
	case *parser.LiteralConstantStringExpressionContext:
		return v.VisitLiteralConstantStringExpression(ctx)
	case *parser.LiteralConstantStringContext:
		return v.VisitLiteralConstantString(ctx)
	case *parser.MultilineStringExpressionContext:
    		return v.VisitMultilineStringExpression(ctx)
    	case *parser.MultilineStringContext:
        	return v.VisitMultilineString(ctx)

	// Binary Operator
	case *parser.BinaryOperatorContext:
		return v.VisitBinaryOperator(ctx)
	case *parser.BinaryOperationExpressionContext:
		return v.VisitBinaryOperationExpression(ctx)

	// Logic
	case *parser.ComparisonOperatorContext:
		return v.VisitComparisonOperator(ctx)
	case *parser.ComparisonExpressionContext:
		return v.VisitComparisonExpression(ctx)
	case *parser.NegatedExpressionContext:
		return v.VisitNegatedExpression(ctx)
	case *parser.LogicalAndExpressionContext:
		return v.VisitLogicalAndExpression(ctx)
	case *parser.LogicalOrExpressionContext:
		return v.VisitLogicalOrExpression(ctx)

	// ifs
	case *parser.IfStatementContext:
		return v.VisitIfStatement(ctx)
	case *parser.IfThenBlockContext:
		return v.VisitIfThenBlock(ctx)
	case *parser.IfElseBlockContext:
		return v.VisitIfElseBlock(ctx)

	// Import
	case *parser.ImportDeclarationContext:
		return v.VisitImportDeclaration(ctx)

	// Attribute
	case *parser.AttributeListContext:
		return v.VisitAttributeList(ctx)
	case *parser.AttributeContext:
		return v.VisitAttribute(ctx)

	// Loop
	case *parser.ForLoopStatementContext:
		return v.VisitForLoopStatement(ctx)
	case *parser.ConditionBasedLoopContext:
		return v.VisitConditionBasedLoop(ctx)
	case *parser.StatementBasedLoopContext:
		return v.VisitStatementBasedLoop(ctx)
	case *parser.ContinueStatementContext:
		return v.VisitContinueStatement(ctx)
	case *parser.BreakStatementContext:
		return v.VisitBreakStatement(ctx)

	case *parser.StructDeclarationContext:
    		return v.VisitStructDeclaration(ctx)
    	case *parser.StructFieldContext:
        	return v.VisitStructField(ctx)
        case *parser.TypeAliasDeclarationContext:
            	return v.VisitTypeAliasDeclaration(ctx)

	default:
		log.Fatalf("ERROR: stepped into an unimplemented context: %s\n", ctx.GetText())
	}

	panic("ERROR: unreachable")
}

func (v *AstVisitor) VisitFile(ctx *parser.FileContext) any {
	for _, st := range ctx.AllStatement() {
		v.Visit(st)
	}

	return nil
}

func (v *AstVisitor) VisitStatement(ctx *parser.StatementContext) any {
	return v.Visit(ctx.GetChild(0).(antlr.ParseTree))
}

func (v *AstVisitor) VisitEmphasizedExpression(ctx *parser.EmphasizedExpressionContext) any {
	return v.Visit(ctx.Expression())
}
