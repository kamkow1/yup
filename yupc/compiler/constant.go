package compiler

import (
	"strconv"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitConstantExpression(ctx *parser.ConstantExpressionContext) any {
	return v.Visit(ctx.Constant())
}

func (v *AstVisitor) VisitStringExpr(ctx *parser.StringExprContext) any {
	var raw string
	if ctx.ValueString() != nil {
		raw, _ = strconv.Unquote(ctx.ValueString().GetText())
	} else if ctx.MultilineString() != nil {
		raw = v.Visit(ctx.MultilineString()).(string)
	}

	return raw
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

	if ctx.StringExpr() != nil {
		strexpr := ctx.StringExpr().(*parser.StringExprContext)
		raw := v.Visit(ctx.StringExpr()).(string)
		if strexpr.KeywordValueString() != nil {
			return raw
		}

		value = CompilationUnits.Peek().Builder.CreateGlobalStringPtr(raw, "")
	}

	if ctx.ValueNull() != nil {
		value = llvm.ConstPointerNull(llvm.PointerType(llvm.Int8Type(), 0))
	}

	return value
}

func (v *AstVisitor) VisitMultilineStringExpression(ctx *parser.MultilineStringExpressionContext) any {
	return v.Visit(ctx.MultilineString())
}

func (v *AstVisitor) VisitMultilineString(ctx *parser.MultilineStringContext) any {
	buf := ctx.GetText()
	buf = buf[1 : len(buf)-1]

	return buf
}
