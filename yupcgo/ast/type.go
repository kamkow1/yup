package ast

import (
	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/kamkow1/yup/yupcgo/parser"
)

func (v *AstVisitor) VisitTypeName(ctx *parser.TypeNameContext) any {
	return compiler.GetTypeFromName(ctx.GetText())
}

func (v *AstVisitor) VisitTypeAnnotation(ctx *parser.TypeAnnotationContext) any {
	return v.Visit(ctx.TypeName())
}
