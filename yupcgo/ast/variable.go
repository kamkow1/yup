package ast

import (
	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/kamkow1/yup/yupcgo/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitVariableValue(ctx *parser.VariableValueContext) any {
	return v.Visit(ctx.Expression())
}

func (v *AstVisitor) VisitDeclarationType(ctx *parser.DeclarationTypeContext) any {
	return ctx.KeywordConst() != nil
}

func (v *AstVisitor) VisitVariableDeclare(ctx *parser.VariableDeclareContext) any {
	name := ctx.Identifier().GetText()
	typ := v.Visit(ctx.TypeAnnotation()).(llvm.Type)
	isGlobal := ctx.KeywordGlobal() != nil
	isExported := ctx.KeywordExport() != nil
	isConstant := v.Visit(ctx.DeclarationType()).(bool) // true == const, false == var

	compiler.CreateVariable(name, typ, isGlobal, isConstant, isExported)

	if ctx.VariableValue() != nil {
		value := v.Visit(ctx.VariableValue()).(llvm.Value)
		compiler.InitializeVariable(name, value, isGlobal)
	}

	return nil
}
