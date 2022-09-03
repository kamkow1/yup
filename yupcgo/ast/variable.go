package ast

import (
	"fmt"

	"github.com/kamkow1/yup/yupcgo/parser"
	"tinygo.org/x/go-llvm"
)

type Variable struct {
	name    string
	isConst bool
}

type LocalVariable struct {
	*Variable
	value *llvm.Value
}

type GlobalVariable struct {
	*Variable
	value *llvm.DIGlobalVariableExpression
}

func (v *AstVisitor) VisitVariableValue(ctx *parser.VariableValueContext) any {
	return nil
}

func (v *AstVisitor) VisitVariableDeclare(ctx *parser.VariableDeclareContext) any {
	fmt.Printf("%s\n", ctx.Identifier().GetText())
	return nil
}
