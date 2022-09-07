package ast

import (
	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/kamkow1/yup/yupcgo/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitAddressOfExpression(ctx *parser.AddressOfExpressionContext) any {
	return v.Visit(ctx.AddressOf())
}

func (v *AstVisitor) VisitAddressOf(ctx *parser.AddressOfContext) any {
	name := ctx.Identifier().GetText()
	if compiler.CompilationUnits.Peek().Module.NamedGlobal(name).IsGlobalConstant() {
		return compiler.CompilationUnits.Peek().Module.NamedGlobal(name)
	}
	/*if global, ok := compiler.CompilationUnits.Peek().Globals[name]; ok {
		return global.Value
	}*/

	return compiler.FindLocalVariable(name, len(compiler.CompilationUnits.Peek().Locals)-1).Value
}

func (v *AstVisitor) VisitPointerDereferenceExpression(ctx *parser.PointerDereferenceExpressionContext) any {
	return v.Visit(ctx.PointerDereference())
}

func (v *AstVisitor) VisitPointerDereference(ctx *parser.PointerDereferenceContext) any {
	ptr := v.Visit(ctx.Expression()).(llvm.Value)
	return compiler.DereferencePointer(ptr, &compiler.CompilationUnits.Peek().Builder)
}
