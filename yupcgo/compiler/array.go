package compiler

import (
	"tinygo.org/x/go-llvm"
)

func CreateArray(vals []llvm.Value) llvm.Value {
	typ := vals[0].Type()
	return llvm.ConstArray(typ, vals)
}

func GetElemByIndex(array llvm.Value, val llvm.Value, builder *llvm.Builder) llvm.Value {
	var indices []llvm.Value

	if array.Type().ElementType().TypeKind() == llvm.ArrayTypeKind {
		indices = append(indices, GetIntegerConstant(0))
	}

	indices = append(indices, val)
	gep := builder.CreateGEP(array, indices, "")
	return DereferencePointer(gep, builder)
}
