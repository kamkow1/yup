package ast

import (
	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/kamkow1/yup/yupcgo/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitVariableValue(ctx *parser.VariableValueContext) any {
	return nil
}

func (v *AstVisitor) VisitVariableDeclare(ctx *parser.VariableDeclareContext) any {
	name := ctx.Identifier().GetText()
	typ := v.Visit(ctx.TypeAnnotation()).(llvm.Type)
	isGlobal := ctx.KeywordGlobal() != nil

	compiler.CreateVariable(name, typ, isGlobal)

	return nil
}
