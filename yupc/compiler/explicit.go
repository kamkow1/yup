package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
)

func (v *AstVisitor) VisitExplicitIdentifierExpression(ctx *parser.ExplicitIdentifierExpressionContext) any {
	return ctx.Identifier().GetText()
}
