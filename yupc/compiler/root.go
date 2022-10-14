package compiler

import (
	"github.com/antlr/antlr4/runtime/Go/antlr"
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

type any = interface{}

type CompilerInfo struct {
	Line         int
	TargetTriple string
	Verbose      bool
	PrintModule  bool
}

func NewCompilerInfo() *CompilerInfo {
	return &CompilerInfo{
		Line:         0,
		TargetTriple: llvm.DefaultTargetTriple(),
		Verbose:      false,
		PrintModule:  false,
	}
}

var GlobalCompilerInfo *CompilerInfo = NewCompilerInfo()

type AstVisitor struct {
	*parser.BaseYupParserVisitor
}

func NewAstVisitor() *AstVisitor {
	return &AstVisitor{}
}

func (v *AstVisitor) Visit(tree antlr.ParseTree) any {
	switch ctx := tree.(type) {
	case *parser.FileContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFile(ctx)
	case *parser.CodeBlockContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitCodeBlock(ctx)
	case *parser.EmphasizedExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitEmphasizedExpression(ctx)
	case *parser.StatementContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStatement(ctx)

	// Type
	case *parser.TypeAnnotationContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitTypeAnnotation(ctx)
	case *parser.TypeNameContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitTypeName(ctx)
	case *parser.TypeExtensionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitTypeExtension(ctx)
	case *parser.ArrayTypeExtensionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitArrayTypeExtension(ctx)
	case *parser.LiteralTypeExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitLiteralTypeExpression(ctx)

	// Array
	//case *parser.ArrayElementAssignmentContext:
	//	GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
	//	return v.VisitArrayElementAssignment(ctx)
	case *parser.ArrayIndexContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitArrayIndex(ctx)
	case *parser.ConstArrayContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConstArray(ctx)
	case *parser.ConstArrayExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConstArrayExpression(ctx)
	case *parser.IndexedAccessExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitIndexedAccessExpression(ctx)

	// Variable
	case *parser.AssignmentContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitAssignment(ctx)
	case *parser.VariableDeclareContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitVariableDeclare(ctx)
	case *parser.DeclarationTypeContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitDeclarationType(ctx)
	case *parser.VariableValueContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitVariableValue(ctx)
	case *parser.IdentifierExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitIdentifierExpression(ctx)

	// Function
	case *parser.FunctionDefinitionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFunctionDefinition(ctx)
	case *parser.FunctionSignatureContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFunctionSignature(ctx)
	case *parser.FunctionParameterListContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFunctionParameterList(ctx)
	case *parser.FunctionParameterContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFunctionParameter(ctx)
	case *parser.FunctionReturnContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFunctionReturn(ctx)
	case *parser.FunctionCallContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFunctionCall(ctx)
	case *parser.FunctionCallArgListContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFunctionCallArgList(ctx)
	case *parser.FunctionCallExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFunctionCallExpression(ctx)

	// Memory
	case *parser.AddressOfContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitAddressOf(ctx)
	case *parser.AddressOfExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitAddressOfExpression(ctx)
	case *parser.PointerDereferenceExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitPointerDereferenceExpression(ctx)
	// case *parser.DereferenceAssignmentContext:
	// GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
	// return v.VisitDereferenceAssignment(ctx)
	case *parser.ExpressionAssignmentContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitExpressionAssignment(ctx)

	// Constant
	case *parser.ConstantContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConstant(ctx)
	case *parser.ConstantExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConstantExpression(ctx)
	case *parser.LiteralConstantIntExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitLiteralConstantIntExpression(ctx)
	case *parser.LiteralConstantIntContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitLiteralConstantInt(ctx)
	case *parser.LiteralConstantStringExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitLiteralConstantStringExpression(ctx)
	case *parser.LiteralConstantStringContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitLiteralConstantString(ctx)
	case *parser.MultilineStringExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitMultilineStringExpression(ctx)
	case *parser.MultilineStringContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitMultilineString(ctx)

	// Binary Operator
	case *parser.BinaryOperatorContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitBinaryOperator(ctx)
	case *parser.BinaryOperationExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitBinaryOperationExpression(ctx)

	// Logic
	case *parser.ComparisonOperatorContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitComparisonOperator(ctx)
	case *parser.ComparisonExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitComparisonExpression(ctx)
	case *parser.NegatedExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitNegatedExpression(ctx)
	case *parser.LogicalAndExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitLogicalAndExpression(ctx)
	case *parser.LogicalOrExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitLogicalOrExpression(ctx)

	// ifs
	case *parser.IfStatementContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitIfStatement(ctx)
	case *parser.IfThenBlockContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitIfThenBlock(ctx)
	case *parser.IfElseBlockContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitIfElseBlock(ctx)

	// Import
	case *parser.ImportDeclarationContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitImportDeclaration(ctx)

	// Attribute
	case *parser.AttributeListContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitAttributeList(ctx)
	case *parser.AttributeContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitAttribute(ctx)

	// Loop
	case *parser.ForLoopStatementContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitForLoopStatement(ctx)
	case *parser.ConditionBasedLoopContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConditionBasedLoop(ctx)
	case *parser.StatementBasedLoopContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStatementBasedLoop(ctx)
	case *parser.ContinueStatementContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitContinueStatement(ctx)
	case *parser.BreakStatementContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitBreakStatement(ctx)

	// Struct
	case *parser.StructDeclarationContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStructDeclaration(ctx)
	case *parser.StructFieldContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStructField(ctx)
	case *parser.TypeAliasDeclarationContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitTypeAliasDeclaration(ctx)
	case *parser.FieldAccessExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFieldAccessExpression(ctx)
	case *parser.FieldAssignmentContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFieldAssignment(ctx)
	case *parser.StructInitExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStructInitExpression(ctx)
	case *parser.StructInitContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStructInit(ctx)
	case *parser.FieldInitContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFieldInit(ctx)
	case *parser.ConstStructInitExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConstStructInitExpression(ctx)
	case *parser.ConstStructInitContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConstStructInit(ctx)
	case *parser.MethodCallExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitMethodCallExpression(ctx)

	default:
		LogError("stepped into an unimplemented context: %s\n", ctx.GetText())
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
