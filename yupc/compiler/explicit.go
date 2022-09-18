package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
)

func (v *AstVisitor) VisitExplicitIdentifierExpression(ctx *parser.ExplicitIdentifierExpressionContext) any {
	return ctx.Identifier().GetText()
}

func (v *AstVisitor) VisitExplicitStringExpression(ctx *parser.ExplicitStringExpressionContext) any {
	str := TrimLeftChar(ctx.ValueString().GetText()) // trim " (1)
	str = TrimRightChar(str)                         // trim " (2)

	return str
}
