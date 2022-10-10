package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitAddressOfExpression(ctx *parser.AddressOfExpressionContext) any {
	return v.Visit(ctx.AddressOf())
}

func (v *AstVisitor) VisitAddressOf(ctx *parser.AddressOfContext) any {
	name := ctx.Identifier().GetText()
	if !CompilationUnits.Peek().Module.NamedFunction(name).IsNil() {
		return CompilationUnits.Peek().Module.NamedFunction(name)
	}

	if !CompilationUnits.Peek().Module.NamedGlobal(name).IsNil() {
		return CompilationUnits.Peek().Module.NamedGlobal(name)
	}

	return FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1).Value
}

func (v *AstVisitor) VisitPointerDereferenceExpression(ctx *parser.PointerDereferenceExpressionContext) any {
	ptr := v.Visit(ctx.Expression()).(llvm.Value)
	return CompilationUnits.Peek().Builder.CreateLoad(ptr, "")
}
