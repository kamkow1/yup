package compiler

import (
	"fmt"

	"tinygo.org/x/go-llvm"
)

var BuiltinLLVMTypes map[string]llvm.Type

func init() {
	BuiltinLLVMTypes = map[string]llvm.Type{
		"i1":   llvm.Int1Type(),
		"i8":   llvm.Int8Type(),
		"i16":  llvm.Int16Type(),
		"i32":  llvm.Int32Type(),
		"i64":  llvm.Int64Type(),
		"f32":  llvm.FloatType(),
		"f128": llvm.FP128Type(),
		"dbl":  llvm.DoubleType(),
		"void": llvm.VoidType(),
	}
}

func GetPointerType(typ llvm.Type) llvm.Type {
	return llvm.PointerType(typ, 0)
}

func GetArrayType(typ llvm.Type, count int) llvm.Type {
	return llvm.ArrayType(typ, count)
}

func GetTypeFromName(name string) llvm.Type {
	if llvmType, ok := BuiltinLLVMTypes[name]; ok {
		return llvmType
	}

	panic(fmt.Sprintf("ERROR: unknown type: %s", name))
}

func AssertType(typ1 llvm.Type, typ2 llvm.Type) bool {
	return typ1.TypeKind() != typ2.TypeKind()
}
