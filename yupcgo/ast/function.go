package ast

import (
	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/kamkow1/yup/yupcgo/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitFunctionParameter(ctx *parser.FunctionParameterContext) any {
	return v.Visit(ctx.TypeAnnotation())
}

func (v *AstVisitor) VisitFunctionParameterList(ctx *parser.FunctionParameterListContext) any {
	var types []llvm.Type
	for _, p := range ctx.AllFunctionParameter() {
		types = append(types, v.Visit(p).(llvm.Type))
	}

	return types
}

func (v *AstVisitor) VisitFunctionSignature(ctx *parser.FunctionSignatureContext) any {
	name := ctx.Identifier().GetText()
	isExported := ctx.KeywordExport() != nil
	var paramTypes []llvm.Type
	if ctx.FunctionParameterList() != nil {
		paramTypes = v.Visit(ctx.FunctionParameterList()).([]llvm.Type)
	} else {
		paramTypes = make([]llvm.Type, 0)
	}

	returnType := v.Visit(ctx.TypeName()).(llvm.Type)
	return compiler.CreateFuncSignature(name, paramTypes, returnType,
		isExported, &compiler.CompilationUnits.Peek().Module)
}

func (v *AstVisitor) VisitFunctionDefinition(ctx *parser.FunctionDefinitionContext) any {
	signature := v.Visit(ctx.FunctionSignature()).(llvm.Value)
	compiler.CreateBlock()
	compiler.CreateFunctionEntryBlock(signature, &compiler.CompilationUnits.Peek().Builder)

	v.Visit(ctx.CodeBlock())
	if signature.Type().ReturnType().ElementType().TypeKind() == llvm.VoidTypeKind {
		return compiler.BuildVoidReturn(&compiler.CompilationUnits.Peek().Builder)
	}

	return nil
}

func (v *AstVisitor) VisitFunctionCallArgList(ctx *parser.FunctionCallArgListContext) any {
	var args []llvm.Value
	for _, expr := range ctx.AllExpression() {
		args = append(args, v.Visit(expr).(llvm.Value))
	}

	return args
}

func (v *AstVisitor) VisitFunctionCallExpression(ctx *parser.FunctionCallExpressionContext) any {
	return v.Visit(ctx.FunctionCall())
}

func (v *AstVisitor) VisitFunctionCall(ctx *parser.FunctionCallContext) any {
	name := ctx.Identifier().GetText()
	var args []llvm.Value
	if ctx.FunctionCallArgList() != nil {
		args = v.Visit(ctx.FunctionCallArgList()).([]llvm.Value)
	} else {
		args = make([]llvm.Value, 0)
	}

	return compiler.CallFunction(name, args, &compiler.CompilationUnits.Peek().Builder)
}

func (v *AstVisitor) VisitFunctionReturn(ctx *parser.FunctionReturnContext) any {
	if ctx.Expression() != nil {
		return compiler.BuildValueReturn(v.Visit(ctx.Expression()).(llvm.Value),
			&compiler.CompilationUnits.Peek().Builder)
	} else {
		return compiler.BuildVoidReturn(&compiler.CompilationUnits.Peek().Builder)
	}
}
