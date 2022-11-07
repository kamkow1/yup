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

func CompareInts(v0 llvm.Value, v1 llvm.Value, compType int) llvm.Value {
	var result llvm.Value
	switch compType {
	case EqualComp:
		result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.IntEQ), v0, v1, "")
	case NotEqualComp:
		result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.IntNE), v0, v1, "")
	case MoreThanComp:
		result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.IntUGT), v0, v1, "")
	case LessThanComp:
		result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.IntULT), v0, v1, "")
	case LessOrEqualComp:
		result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.IntULE), v0, v1, "")
	case MoreOrEqualComp:
		result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.IntUGE), v0, v1, "")
	}

	return result
}

func CompareNonInts(v0 llvm.Value, v1 llvm.Value, compType int) llvm.Value {
	var result llvm.Value
	switch compType {
	case EqualComp:
		result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.FloatUEQ), v0, v1, "")
	case NotEqualComp:
		result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.FloatUNE), v0, v1, "")
	case MoreThanComp:
		result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.FloatUGT), v0, v1, "")
	case LessThanComp:
		result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.FloatULT), v0, v1, "")
	case LessOrEqualComp:
		result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.FloatULE), v0, v1, "")
	case MoreOrEqualComp:
		result = CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.FloatUGE), v0, v1, "")
	}

	return result
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

	var value llvm.Value
	op := ctx.CompOper().(*parser.CompOperContext)
	if op.SymbolEqual() != nil {
		if v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind {
			value = CompareInts(v0, v1, EqualComp)
		} else {
			v0 = CastIntToFloatIfNeeded(v0)
			v1 = CastIntToFloatIfNeeded(v1)
			value = CompareNonInts(v0, v1, EqualComp)
		}
	} else if op.SymbolNotEqual() != nil {
		if v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind {
			value = CompareInts(v0, v1, NotEqualComp)
		} else {
			v0 = CastIntToFloatIfNeeded(v0)
			v1 = CastIntToFloatIfNeeded(v1)
			value = CompareNonInts(v0, v1, NotEqualComp)
		}
	} else if op.SymbolMoreThan() != nil {
		if v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind {
			value = CompareInts(v0, v1, MoreThanComp)
		} else {
			v0 = CastIntToFloatIfNeeded(v0)
			v1 = CastIntToFloatIfNeeded(v1)
			value = CompareInts(v0, v1, MoreThanComp)
		}
	} else if op.SymbolLessThan() != nil {
		if v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind {
			value = CompareInts(v0, v1, LessThanComp)
		} else {
			v0 = CastIntToFloatIfNeeded(v0)
			v1 = CastIntToFloatIfNeeded(v1)
			value = CompareNonInts(v0, v1, LessThanComp)
		}
	} else if op.SymbolLessOrEqual() != nil {
		if v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind {
			value = CompareInts(v0, v1, LessOrEqualComp)
		} else {
			v0 = CastIntToFloatIfNeeded(v0)
			v1 = CastIntToFloatIfNeeded(v1)
			value = CompareNonInts(v0, v1, LessOrEqualComp)
		}
	} else if op.SymbolMoreOrEqual() != nil {
		if v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind {
			value = CompareNonInts(v0, v1, MoreOrEqualComp)
		} else {
			v0 = CastIntToFloatIfNeeded(v0)
			v1 = CastIntToFloatIfNeeded(v1)
			value = CompareNonInts(v0, v1, MoreOrEqualComp)
		}
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
