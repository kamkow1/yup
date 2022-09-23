package compiler

import (
	"encoding/ascii85"
	"log"
	"strconv"
	"strings"

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
			log.Fatalf("ERROR: unable to parse int: %s", err)
		}

		value = llvm.ConstInt(llvm.Int32Type(), uint64(i), false)
	}

	if ctx.ValueFloat() != nil {
		str := ctx.ValueFloat().GetText()
		f, err := strconv.ParseFloat(str, 64)
		if err != nil {
			log.Fatalf("ERROR: unable to parse float: %s", err)
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

		newString := make([]string, 0)
		i := 0
		for i < len(str) {
			if (i != len(str)-1) && (str[i] == '\\') && (str[i+1] == 'n') {
				newString = append(newString, "\\0A")
				i += 2
			} else {
				newString = append(newString, string(str[i]))
				i++
			}
		}

		str = strings.Join(newString, "")

		value = CompilationUnits.Peek().Builder.CreateGlobalStringPtr(str, "")
	}

	if ctx.ValueBool() != nil {
		var b int
		if ctx.ValueBool().GetText() == "True" {
			b = 1
		} else {
			b = 0
		}

		value = llvm.ConstInt(llvm.Int1Type(), uint64(b), false)
	}

	if ctx.ValueNull() != nil {
		value = llvm.ConstPointerNull(llvm.PointerType(llvm.Int8Type(), 0))
	}

	return value
}

func (v *AstVisitor) VisitLiteralConstantIntExpression(ctx *parser.LiteralConstantIntExpressionContext) any {
	return v.Visit(ctx.LiteralConstantInt())
}

func (v *AstVisitor) VisitLiteralConstantInt(ctx *parser.LiteralConstantIntContext) any {
	str := ctx.ValueInteger().GetText()
	i, err := strconv.ParseInt(str, 10, 64)
	if err != nil {
		log.Fatalf("ERROR: unable to parse int: %s", err)
	}

	return i
}
