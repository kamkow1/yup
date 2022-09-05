package ast

import (
	"encoding/ascii85"
	"fmt"
	"strconv"

	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/kamkow1/yup/yupcgo/parser"
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
			//panic(fmt.Sprintf("ERROR: unable to parse int: %s", err))
			//panic(logging.LogMessage(fmt.Sprintf("unable to parse int: %s", err), cmd.Options.VerboseMode))
		}

		value = compiler.GetIntegerConstant(i)
	}

	if ctx.ValueFloat() != nil {
		str := ctx.ValueFloat().GetText()
		f, err := strconv.ParseFloat(str, 64)
		if err != nil {
			panic(fmt.Sprintf("ERROR: unable to parse float: %s", err))
		}

		value = compiler.GetFloatConstant(f)
	}

	if ctx.ValueChar() != nil {
		str := ctx.ValueChar().GetText()
		c := str[:(len(str)+1)-(len(str)-1)]

		dst := make([]byte, 8)
		value = compiler.GetCharConstant(byte(ascii85.Encode(dst, []byte(c))))
	}

	if ctx.ValueString() != nil {
		value = compiler.GetStringConstant(ctx.ValueString().GetText())
	}

	return value
}
