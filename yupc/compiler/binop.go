package compiler

import (
	"log"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitBinaryOperationExpression(ctx *parser.BinaryOperationExpressionContext) any {
	v0 := v.Visit(ctx.Expression(0)).(llvm.Value)
	v1 := v.Visit(ctx.Expression(1)).(llvm.Value)

	if AssertType(v0.Type(), v1.Type()) {
		log.Fatalf("ERROR: cannot perform binary operation on different types: %s & %s",
			v0.Type().String(), v1.Type().String())
	}

	binop := ctx.BinaryOperator().(*parser.BinaryOperatorContext)

	if binop.SymbolPlus() != nil {
		return CompilationUnits.Peek().Builder.CreateAdd(v0, v1, "")
	} else if binop.SymbolMinus() != nil {
		return CompilationUnits.Peek().Builder.CreateSub(v0, v1, "")
	} else if binop.SymbolAsterisk() != nil {
		return CompilationUnits.Peek().Builder.CreateMul(v0, v1, "")
	} else {
		return CompilationUnits.Peek().Builder.CreateFDiv(v0, v1, "")
	}
}
