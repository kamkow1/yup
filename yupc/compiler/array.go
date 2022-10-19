package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

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

func IsArray(a llvm.Value) bool {
	return a.Type().ElementType().TypeKind() == llvm.ArrayTypeKind
}

func IsPointer(a llvm.Value) bool {
	return a.Type().ElementType().TypeKind() == llvm.PointerTypeKind
}

/*func (v *AstVisitor) VisitArrayElementAssignment(ctx *parser.ArrayElementAssignmentContext) any {
	name := ctx.Identifier().GetText()
	array := FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1).Value
	value := v.Visit(ctx.VariableValue()).(llvm.Value)

	for _, i := range ctx.AllArrayIndex() {
		idx := v.Visit(i.(*parser.ArrayIndexContext).Expression()).(llvm.Value)

		var indices []llvm.Value
		if IsArray(array) {
			i64_0 := llvm.ConstInt(llvm.Int64Type(), 0, false)
			indices = append(indices, i64_0)
		}

		indices = append(indices, idx)

		if IsPointer(array) {
			deref := CompilationUnits.Peek().Builder.CreateLoad(array, "")
			array = CompilationUnits.Peek().Builder.CreateGEP(deref, indices, "")
		} else {
			array = CompilationUnits.Peek().Builder.CreateGEP(array, indices, "")
		}
	}

	CompilationUnits.Peek().Builder.CreateStore(value, array)

	return nil
}*/
