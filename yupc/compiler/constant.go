package compiler

import (
	"encoding/ascii85"
	"encoding/binary"
	"fmt"
	"strconv"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitConstantExpression(ctx *parser.ConstantExpressionContext) any {
	return v.Visit(ctx.Constant())
}

func (v *AstVisitor) VisitConstant(ctx *parser.ConstantContext) any {

	var value llvm.Value
	if ctx.ValueInteger() != nil {
		str := ctx.ValueInteger().GetText()
		i, err := strconv.ParseInt(str, 10, 64)
		if err != nil {
			panic(fmt.Sprintf("ERROR: unable to parse int: %s", err))
		}

		size := binary.Size(i)
		typ := llvm.IntType(int(size))
		value = llvm.ConstInt(typ, uint64(i), false)
	}

	if ctx.ValueFloat() != nil {
		str := ctx.ValueFloat().GetText()
		f, err := strconv.ParseFloat(str, 64)
		if err != nil {
			panic(fmt.Sprintf("ERROR: unable to parse float: %s", err))
		}

		typ := llvm.FloatType()
		value = llvm.ConstFloat(typ, f)
	}

	if ctx.ValueChar() != nil {
		str := ctx.ValueChar().GetText()
		c := str[:(len(str)+1)-(len(str)-1)]

		dst := make([]byte, 8)
		typ := llvm.Int8Type()
		enc := ascii85.Encode(dst, []byte(c))
		value = llvm.ConstInt(typ, uint64(byte(enc)), false)
	}

	if ctx.ValueString() != nil {
		str := ctx.ValueString().GetText()
		str = TrimLeftChar(str)
		str = TrimRightChar(str)
		value = llvm.ConstString(str, true)
	}

	return value
}

func TrimLeftChar(s string) string {
	for i := range s {
		if i > 0 {
			return s[i:]
		}
	}
	return s[:0]
}

func TrimRightChar(s string) string {
	return s[:len(s)-1]
}
