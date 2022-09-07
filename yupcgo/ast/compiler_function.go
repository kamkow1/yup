package ast

import (
	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/kamkow1/yup/yupcgo/parser"
)

func (v *AstVisitor) VisitYupFunctionExpression(ctx *parser.YupFunctionExpressionContext) any {
	return v.Visit(ctx.YupFunction())
}

func (v *AstVisitor) VisitYupFunction(ctx *parser.YupFunctionContext) any {
	callCtx := ctx.FunctionCall().(*parser.FunctionCallContext)
	name := callCtx.Identifier().GetText()
	var strArgs []string
	args := callCtx.FunctionCallArgList().(*parser.FunctionCallArgListContext)
	for _, a := range args.AllExpression() {
		strArgs = append(strArgs, a.GetText())
	}

	return compiler.CallBuiltinFunction(name, strArgs)
}
