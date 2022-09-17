package compiler

import (
	"encoding/ascii85"
	"encoding/binary"
	"log"
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
			log.Fatalf("ERROR: unable to parse int: %s", err)
		}

		size := binary.Size(i)
		typ := llvm.IntType(int(size))
		value = llvm.ConstInt(typ, uint64(i), false)
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

		for _, p := range ctx.AllStringPrefix() {
			pref := p.(*parser.StringPrefixContext)
			if pref.KeywordGStrPrefix() != nil {
				value = CompilationUnits.Peek().Builder.CreateGlobalStringPtr(str, "")
			}

			if pref.KeywordLocalStrPrefix() != nil {
				value = llvm.ConstString(str, true)
			}
		}
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
