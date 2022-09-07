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
	var params []compiler.FuncParam
	for _, p := range ctx.AllFunctionParameter() {
		params = append(params, compiler.FuncParam{
			Name: p.(*parser.FunctionParameterContext).Identifier().GetText(),
			Type: v.Visit(p).(llvm.Type)})
	}

	return params
}

func (v *AstVisitor) VisitFunctionSignature(ctx *parser.FunctionSignatureContext) any {
	name := ctx.Identifier().GetText()
	var paramTypes []compiler.FuncParam
	if ctx.FunctionParameterList() != nil {
		paramTypes = v.Visit(ctx.FunctionParameterList()).([]compiler.FuncParam)
	} else {
		paramTypes = make([]compiler.FuncParam, 0)
	}

	returnType := v.Visit(ctx.TypeName()).(llvm.Type)
	f := compiler.CreateFuncSignature(name, paramTypes,
		returnType, &compiler.CompilationUnits.Peek().Module)

	for i, pt := range paramTypes {
		f.Param(i).SetName(pt.Name)
	}

	return f
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
