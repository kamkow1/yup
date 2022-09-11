package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitArrayExpression(ctx *parser.ArrayExpressionContext) any {
	return v.Visit(ctx.Array())
}

func (v *AstVisitor) VisitArray(ctx *parser.ArrayContext) any {
	var vals []llvm.Value
	for _, expr := range ctx.AllExpression() {
		val := v.Visit(expr).(llvm.Value)
		vals = append(vals, val)
	}

	typ := vals[0].Type()
	return llvm.ConstArray(typ, vals)
}

func (v *AstVisitor) VisitIndexedAccessExpression(ctx *parser.IndexedAccessExpressionContext) any {
	var idx llvm.Value
	for i := 1; i < len(ctx.AllExpression()); i++ {
		array := v.Visit(ctx.Expression(i - 1)).(llvm.Value)
		val := v.Visit(ctx.Expression(i)).(llvm.Value)

		var indices []llvm.Value
		if array.Type().ElementType().TypeKind() == llvm.ArrayTypeKind {
			indices = append(indices, llvm.ConstInt(llvm.Int64Type(), 0, false))
		}

		indices = append(indices, val)
		gep := CompilationUnits.Peek().Builder.CreateGEP(array, indices, "")
		idx = CompilationUnits.Peek().Builder.CreateLoad(gep, "")
	}

	return idx
}

func (v *AstVisitor) VisitArrayElementAssignment(ctx *parser.ArrayElementAssignmentContext) any {
	name := ctx.Identifier().GetText()
	array := FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1).Value
	var idx llvm.Value
	for _, i := range ctx.AllArrayIndex() {
		idx = v.Visit(i.(*parser.ArrayIndexContext).Expression()).(llvm.Value)
	}

	value := v.Visit(ctx.VariableValue()).(llvm.Value)

	var indices []llvm.Value
	if array.Type().ElementType().TypeKind() == llvm.ArrayTypeKind {
		indices = append(indices, llvm.ConstInt(llvm.Int64Type(), 0, false))
	}

	indices = append(indices, idx)
	if array.Type().ElementType().TypeKind() == llvm.PointerTypeKind {
		deref := CompilationUnits.Peek().Builder.CreateLoad(array, "")
		gep := CompilationUnits.Peek().Builder.CreateGEP(deref, indices, "")
		CompilationUnits.Peek().Builder.CreateStore(value, gep)
	} else {
		gep := CompilationUnits.Peek().Builder.CreateGEP(array, indices, "")
		CompilationUnits.Peek().Builder.CreateStore(value, gep)
	}

	return nil
}
