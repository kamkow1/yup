package compiler

import (
	"log"
	"strconv"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitRangeExpression(ctx *parser.RangeExpressionContext) any {
	bound0, err0 := strconv.ParseInt(ctx.ValueInteger(0).GetText(), 10, 64)
	bound1, err1 := strconv.ParseInt(ctx.ValueInteger(1).GetText(), 10, 64)

	if err0 != nil {
		log.Fatalf("ERROR: failed to parse lower bound of range expr: %v", err0)
	}

	if err1 != nil {
		log.Fatalf("ERROR: failed to parse upper bound of range expr: %v", err1)
	}

	var elems []llvm.Value
	for i := bound0; i <= bound1; i++ {
		val := llvm.ConstInt(llvm.Int64Type(), uint64(i), false)
		elems = append(elems, val)
	}

	return llvm.ConstArray(llvm.Int64Type(), elems)
}
