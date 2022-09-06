package ast

import (
	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/kamkow1/yup/yupcgo/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitArrayExpression(ctx *parser.ArrayExpressionContext) any {
	return v.Visit(ctx.Array())
}

func (v *AstVisitor) VisitArray(ctx *parser.ArrayContext) any {
	var vals []llvm.Value
	for _, expr := range ctx.AllExpression() {
		val := v.Visit(expr).(llvm.Value)
		vals = append(vals, val)
	}

	return compiler.CreateArray(vals)
}

func (v *AstVisitor) VisitIndexedAccessExpression(ctx *parser.IndexedAccessExpressionContext) any {
	var idx llvm.Value
	for i := 1; i < len(ctx.AllExpression()); i++ {
		array := v.Visit(ctx.Expression(i - 1)).(llvm.Value)
		val := v.Visit(ctx.Expression(i)).(llvm.Value)
		idx = compiler.GetElemByIndex(array, val, &compiler.CompilationUnits.Peek().Builder)
	}

	return idx
}
