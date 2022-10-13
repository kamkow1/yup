package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func CastIntToFloatIfNeeded(v0 llvm.Value, v1 llvm.Value) (llvm.Value, llvm.Value) {
	if v0.Type().TypeKind() == llvm.IntegerTypeKind {
		v0 = CompilationUnits.Peek().Builder.CreateSIToFP(v0, llvm.FloatType(), "")
	}

	if v1.Type().TypeKind() == llvm.IntegerTypeKind {
		v1 = CompilationUnits.Peek().Builder.CreateSIToFP(v1, llvm.FloatType(), "")
	}

	return v0, v1
}

const (
	EqualComp       int = 0
	NotEqualComp    int = 1
	LessThanComp    int = 2
	MoreThanComp    int = 3
	LessOrEqualComp int = 4
	MoreOrEqualComp int = 5
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

func (v *AstVisitor) VisitComparisonExpression(ctx *parser.ComparisonExpressionContext) any {
	v0 := v.Visit(ctx.Expression(0)).(llvm.Value)
	v1 := v.Visit(ctx.Expression(1)).(llvm.Value)

	if v0.Type() != v1.Type() {
		v1 = Cast(v1, v0.Type())
	}

	v1tk := v1.Type().TypeKind()
	v0tk := v0.Type().TypeKind()

	var value llvm.Value
	op := ctx.ComparisonOperator().(*parser.ComparisonOperatorContext)
	if op.SymbolEqual() != nil {
		if v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind {
			value = CompareInts(v0, v1, EqualComp)
		} else {
			v0, v1 = CastIntToFloatIfNeeded(v0, v1)
			value = CompareNonInts(v0, v1, EqualComp)
		}
	} else if op.SymbolNotEqual() != nil {
		if v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind {
			value = CompareInts(v0, v1, NotEqualComp)
		} else {
			v0, v1 = CastIntToFloatIfNeeded(v0, v1)
			value = CompareNonInts(v0, v1, NotEqualComp)
		}
	} else if op.SymbolMoreThan() != nil {
		if v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind {
			value = CompareInts(v0, v1, MoreThanComp)
		} else {
			v0, v1 = CastIntToFloatIfNeeded(v0, v1)
			value = CompareInts(v0, v1, MoreThanComp)
		}
	} else if op.SymbolLessThan() != nil {
		if v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind {
			value = CompareInts(v0, v1, LessThanComp)
		} else {
			v0, v1 = CastIntToFloatIfNeeded(v0, v1)
			value = CompareNonInts(v0, v1, LessThanComp)
		}
	} else if op.SymbolLessOrEqual() != nil {
		if v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind {
			value = CompareInts(v0, v1, LessOrEqualComp)
		} else {
			v0, v1 = CastIntToFloatIfNeeded(v0, v1)
			value = CompareNonInts(v0, v1, LessOrEqualComp)
		}
	} else if op.SymbolMoreOrEqual() != nil {
		if v0tk == llvm.IntegerTypeKind && v1tk == llvm.IntegerTypeKind {
			value = CompareNonInts(v0, v1, MoreOrEqualComp)
		} else {
			v0, v1 = CastIntToFloatIfNeeded(v0, v1)
			value = CompareNonInts(v0, v1, MoreOrEqualComp)
		}
	}

	return value
}

func (v *AstVisitor) VisitNegatedExpression(ctx *parser.NegatedExpressionContext) any {
	expr := v.Visit(ctx.Expression()).(llvm.Value)
	ne := CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.IntNE),
		expr, llvm.ConstInt(expr.Type(), 0, false), "")
	xor := CompilationUnits.Peek().Builder.CreateXor(ne,
		llvm.ConstInt(llvm.Int1Type(), 1, false), "")

	return xor
}

func (v *AstVisitor) VisitLogicalAndExpression(ctx *parser.LogicalAndExpressionContext) any {
	v0 := v.Visit(ctx.Expression(0)).(llvm.Value)
	v1 := v.Visit(ctx.Expression(1)).(llvm.Value)

	return CompilationUnits.Peek().Builder.CreateMul(v0, v1, "")
}

func (v *AstVisitor) VisitLogicalOrExpression(ctx *parser.LogicalOrExpressionContext) any {
	v0 := v.Visit(ctx.Expression(0)).(llvm.Value)
	v1 := v.Visit(ctx.Expression(1)).(llvm.Value)

	return CompilationUnits.Peek().Builder.CreateAdd(v0, v1, "")
}
