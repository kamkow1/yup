package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
)

func (v *AstVisitor) VisitImportDeclaration(ctx *parser.ImportDeclarationContext) any {
	name := ctx.ValueString().GetText()
	name = TrimLeftChar(name)
	name = TrimRightChar(name)
	ImportModule(name)
	return nil
}
