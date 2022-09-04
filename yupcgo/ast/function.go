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
	var paramTypes []llvm.Type
	if ctx.FunctionParameterList() != nil {
		paramTypes = v.Visit(ctx.FunctionParameterList()).([]llvm.Type)
	} else {
		paramTypes = make([]llvm.Type, 0)
	}

	returnType := v.Visit(ctx.TypeName()).(llvm.Type)
	return compiler.CreateFuncSignature(name, paramTypes, returnType)
}

func (v *AstVisitor) VisitFunctionDefinition(ctx *parser.FunctionDefinitionContext) any {
	signature := v.Visit(ctx.FunctionSignature()).(llvm.Value)
	compiler.CreateBlock()
	if signature.ParamsCount() > 0 {
		compiler.CreateFunctionEntryBlock(signature)
	}

	v.Visit(ctx.CodeBlock())
	compiler.DebugPrintModule()

	return nil
}
