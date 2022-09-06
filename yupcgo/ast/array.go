package ast

import (
	"fmt"

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
		fmt.Println(val.Type().String())
		vals = append(vals, val)
	}

	return compiler.CreateArray(vals)
}
