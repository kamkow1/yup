package ast

import (
	"github.com/antlr/antlr4/runtime/Go/antlr"
	"github.com/kamkow1/yup/yupcgo/parser"
)

type AstVisitor struct {
	*parser.BaseYupParserVisitor
}

func NewAstVisitor() *AstVisitor {
	return &AstVisitor{}
}

func (v *AstVisitor) Visit(tree antlr.ParseTree) any {
	return tree.Accept(v)
}

func (v *AstVisitor) VisitFile(ctx *parser.FileContext) any {
	for _, st := range ctx.AllStatement() {
		v.Visit(st)
	}

	return nil
}
