package compiler

import (
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
			LogError("unable to parse int: %s", err)
		}

		var typ llvm.Type
		if ctx.TypeAnnot() != nil {
			typ = v.Visit(ctx.TypeAnnot()).(llvm.Type)
		} else {
			typ = llvm.Int32Type()
		}
		value = llvm.ConstInt(typ, uint64(i), false)
	}

	if ctx.ValueFloat() != nil {
		str := ctx.ValueFloat().GetText()
		f, err := strconv.ParseFloat(str, 64)
		if err != nil {
			LogError("unable to parse float: %s", err)
		}

		value = llvm.ConstFloat(llvm.FloatType(), f)
	}

	if ctx.ValueChar() != nil {
		str := ctx.ValueChar().GetText()
		c, _ := strconv.Unquote(str)

		value = llvm.ConstInt(llvm.Int8Type(), uint64(c[0]), false)
	}

	if ctx.MultilineString() != nil {
		raw := v.Visit(ctx.MultilineString()).(string)
		value = CompilationUnits.Peek().Builder.CreateGlobalStringPtr(raw, "")
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
		LogError("unable to parse int: %s", err)
	}

	return i
}

func (v *AstVisitor) VisitLiteralConstantStringExpression(ctx *parser.LiteralConstantStringExpressionContext) any {
	return v.Visit(ctx.LiteralConstantString())
}

func (v *AstVisitor) VisitLiteralConstantString(ctx *parser.LiteralConstantStringContext) any {
	return v.Visit(ctx.MultilineString()).(string)
}

func (v *AstVisitor) VisitMultilineStringExpression(ctx *parser.MultilineStringExpressionContext) any {
	return v.Visit(ctx.MultilineString())
}

func (v *AstVisitor) VisitMultilineString(ctx *parser.MultilineStringContext) any {
	var buf string
	for i, s := range ctx.AllValueString() {
		str, _ := strconv.Unquote(s.GetText())
		if i != len(ctx.AllValueString())-1 {
			str += "\n"
		}

		buf += str
	}

	return buf
}
