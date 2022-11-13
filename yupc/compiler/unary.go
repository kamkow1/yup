package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitBinopExpr(ctx *parser.BinopExprContext) any {
	v0 := v.Visit(ctx.Expression(0)).(llvm.Value)
	v1 := v.Visit(ctx.Expression(1)).(llvm.Value)

	if v0.Type() != v1.Type() {
		v1 = Cast(v1, &TypeInfo{
			Type: v0.Type(),
		})
	}

	binop := ctx.Binop().(*parser.BinopContext)

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

func CastIntToFloatIfNeeded(v0 llvm.Value) llvm.Value {
	if v0.Type().TypeKind() == llvm.IntegerTypeKind {
		v0 = CompilationUnits.Peek().Builder.CreateSIToFP(v0, llvm.FloatType(), "")
	}

	return v0
}

const (
	EqualComp       int = iota
	NotEqualComp        = iota
	LessThanComp        = iota
	MoreThanComp        = iota
	LessOrEqualComp     = iota
	MoreOrEqualComp     = iota
)

func CompareInts(v0 llvm.Value, v1 llvm.Value, compType int, signed bool) llvm.Value {
	var pred llvm.IntPredicate
	switch compType {
	case EqualComp:
		pred = llvm.IntEQ
	case NotEqualComp:
		pred = llvm.IntNE
	case MoreThanComp:
		if signed {
			pred = llvm.IntSGT
		} else {
			pred = llvm.IntUGT
		}
	case LessThanComp:
		if signed {
			pred = llvm.IntSLT
		} else {
			pred = llvm.IntULT
		}
	case LessOrEqualComp:
		if signed {
			pred = llvm.IntSLE
		} else {
			pred = llvm.IntULE
		}
	case MoreOrEqualComp:
		if signed {
			pred = llvm.IntSGE
		} else {
			pred = llvm.IntUGE
		}
	}

	return CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(pred), v0, v1, "")
}

func CompareNonInts(v0 llvm.Value, v1 llvm.Value, compType int, signed bool) llvm.Value {
	var pred llvm.FloatPredicate
	switch compType {
	case EqualComp:
		//result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.FloatUEQ), v0, v1, "")
		if signed {
			pred = llvm.FloatOEQ
		} else {
			pred = llvm.FloatUEQ
		}
	case NotEqualComp:
		if signed {
			pred = llvm.FloatONE
		} else {
			pred = llvm.FloatUNE
		}
	case MoreThanComp:
		if signed {
			pred = llvm.FloatOGT
		} else {
			pred = llvm.FloatUGT
		}
	case LessThanComp:
		if signed {
			pred = llvm.FloatOLT
		} else {
			pred = llvm.FloatULT
		}
	case LessOrEqualComp:
		if signed {
			pred = llvm.FloatOLE
		} else {
			pred = llvm.FloatULE
		}
	case MoreOrEqualComp:
		if signed {
			pred = llvm.FloatOGE
		} else {
			pred = llvm.FloatUGE
		}
	}

	return CompilationUnits.Peek().Builder.CreateFCmp(llvm.FloatPredicate(pred), v0, v1, "")
}

func (v *AstVisitor) VisitCompExpr(ctx *parser.CompExprContext) any {
	v0 := v.Visit(ctx.Expression(0)).(llvm.Value)
	v1 := v.Visit(ctx.Expression(1)).(llvm.Value)

	if v0.Type() != v1.Type() {
		v1 = Cast(v1, &TypeInfo{
			Type: v0.Type(),
		})
	}

	v1tk := v1.Type().TypeKind()
	v0tk := v0.Type().TypeKind()

	isIntegerCmp := v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind

	var value llvm.Value
	op := ctx.CompOper().(*parser.CompOperContext)
	signed := op.KeywordUnsig() == nil
	var compType int

	if op.SymbolEqual() != nil {
		compType = EqualComp
	}

	if op.SymbolNotEqual() != nil {
		compType = NotEqualComp
	}

	if op.SymbolMoreThan() != nil {
		compType = MoreThanComp
	}

	if op.SymbolLessThan() != nil {
		compType = LessThanComp
	}

	if op.SymbolLessOrEqual() != nil {
		compType = LessOrEqualComp
	}

	if op.SymbolMoreOrEqual() != nil {
		compType = MoreOrEqualComp
	}

	if isIntegerCmp || v1.IsNull() {
		value = CompareInts(v0, v1, compType, signed)
	} else {
		v0 = CastIntToFloatIfNeeded(v0)
		v1 = CastIntToFloatIfNeeded(v1)
		value = CompareNonInts(v0, v1, compType, signed)
	}

	return value
}

func (v *AstVisitor) VisitNegatedExpression(ctx *parser.NegatedExpressionContext) any {
	expr := v.Visit(ctx.Expression()).(llvm.Value)
	if expr.Type() != llvm.Int1Type() {
		expr = Cast(expr, &TypeInfo{
			Type: llvm.Int1Type(),
		})
	}

	ne := CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.IntNE),
		expr, llvm.ConstInt(expr.Type(), 0, false), "")
	xor := CompilationUnits.Peek().Builder.CreateXor(ne,
		llvm.ConstInt(llvm.Int1Type(), 1, false), "")

	return xor
}

func (v *AstVisitor) VisitLogicalAndExpression(ctx *parser.LogicalAndExpressionContext) any {
	v0 := v.Visit(ctx.Expression(0)).(llvm.Value)
	v1 := v.Visit(ctx.Expression(1)).(llvm.Value)

	if v0.Type() != v1.Type() {
		v1 = Cast(v1, &TypeInfo{
			Type: v0.Type(),
		})
	}

	return CompilationUnits.Peek().Builder.CreateAnd(v0, v1, "")
}

func (v *AstVisitor) VisitLogicalOrExpression(ctx *parser.LogicalOrExpressionContext) any {
	v0 := v.Visit(ctx.Expression(0)).(llvm.Value)
	v1 := v.Visit(ctx.Expression(1)).(llvm.Value)

	if v0.Type() != v1.Type() {
		v1 = Cast(v1, &TypeInfo{
			Type: v0.Type(),
		})
	}

	return CompilationUnits.Peek().Builder.CreateOr(v0, v1, "")
}

func (v *AstVisitor) VisitIncremDecremExpr(ctx *parser.IncremDecremExprContext) any {
	name := ctx.Identifier().GetText()
	value := FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1).Value
	ptr := value

	if value.Type().TypeKind() == llvm.PointerTypeKind {
		value = CompilationUnits.Peek().Builder.CreateLoad(value.Type().ElementType(), value, "")
	} else {
		LogError("cannot increment/decrement non-pointer variables")
	}

	if value.Type().TypeKind() != llvm.IntegerTypeKind {
		value = Cast(value, &TypeInfo{
			Type: llvm.Int64Type(),
		})
	}

	var newValue llvm.Value
	if ctx.SymbolIncrement() != nil {
		one := llvm.ConstInt(value.Type(), uint64(1), true)
		newValue = CompilationUnits.Peek().Builder.CreateAdd(value, one, "")
	} else if ctx.SymbolDecrement() != nil {
		negone := llvm.ConstIntFromString(value.Type(), "-1", -1)
		newValue = CompilationUnits.Peek().Builder.CreateSub(value, negone, "")
	}

	CompilationUnits.Peek().Builder.CreateStore(newValue, ptr)
	return value
}
