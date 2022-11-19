package compiler

import (
	"strconv"
	"strings"

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

		var typ *TypeInfo
		if ctx.TypeAnnot() != nil {
			typ = v.Visit(ctx.TypeAnnot()).(*TypeInfo)
		} else {
			typ = &TypeInfo{
				Type: llvm.Int32Type(),
			}
		}

		value = llvm.ConstInt(typ.Type, uint64(i), ctx.KeywordUnsig() == nil)
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
		value = llvm.ConstNull(v.Visit(ctx.TypeAnnot()).(*TypeInfo).Type)
	}

	if ctx.ValueNullptr() != nil {
		value = llvm.ConstPointerNull(v.Visit(ctx.TypeAnnot()).(*TypeInfo).Type)
	}

	return value
}

func (v *AstVisitor) VisitMultilineStringExpression(ctx *parser.MultilineStringExpressionContext) any {
	return v.Visit(ctx.MultilineString())
}

func (v *AstVisitor) VisitMultilineString(ctx *parser.MultilineStringContext) any {
	//buf := ctx.GetText()
	//buf = buf[1 : len(buf)-1]
	var elems []string
	for _, str := range ctx.AllValueString() {
		value := str.GetText()
		value = value[1 : len(value)-1]
		elems = append(elems, value)
	}

	return strings.Join(elems, "\n")
}

func (v *AstVisitor) VisitConstArray(ctx *parser.ConstArrayContext) any {
	var vals []llvm.Value
	for _, expr := range ctx.AllExpression() {
		val := v.Visit(expr).(llvm.Value)
		if !val.IsConstant() {
			LogError("expression `%s` is not a constant", expr.GetText())
		}

		vals = append(vals, val)
	}

	typ := vals[0].Type()
	arrtyp := llvm.ArrayType(typ, len(vals))
	return llvm.ConstArray(arrtyp, vals)
}

func (v *AstVisitor) VisitIndexedAccessExpr(ctx *parser.IndexedAccessExprContext) any {
	var idx llvm.Value
	for i := 1; i < len(ctx.AllExpression()); i++ {
		array := v.Visit(ctx.Expression(i - 1)).(llvm.Value)
		val := v.Visit(ctx.Expression(i)).(llvm.Value)

		var indices []llvm.Value
		if array.Type().ElementType().TypeKind() == llvm.ArrayTypeKind {
			indices = append(indices, llvm.ConstInt(llvm.Int64Type(), 0, false))
		}

		indices = append(indices, val)
		idx = CompilationUnits.Peek().Builder.CreateGEP(array.Type().ElementType(), array, indices, "")
	}

	return idx
}
