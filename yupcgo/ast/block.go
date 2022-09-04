package ast

import (
	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/kamkow1/yup/yupcgo/parser"
)

func (v *AstVisitor) VisitCodeBlock(ctx *parser.CodeBlockContext) any {
	compiler.CreateBlock()
	for _, st := range ctx.AllStatement() {
		v.Visit(st)
	}

	return compiler.RemoveBlock()
}
