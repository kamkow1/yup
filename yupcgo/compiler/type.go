package compiler

import (
	"strings"

	"tinygo.org/x/go-llvm"
)

var builtinLLVMTypes map[string]llvm.Type

func init() {
	builtinLLVMTypes = map[string]llvm.Type{
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

func GetTypeFromName(name string) llvm.Type {
	if llvmType, ok := builtinLLVMTypes[name]; ok {
		return llvmType
	} else {
		baseStr := name
		baseStr = strings.ReplaceAll(baseStr, "*", "")
		baseType := GetTypeFromName(baseStr)

		suffixes := name
		suffixes = strings.ReplaceAll(suffixes, baseStr, "")
		for _, s := range suffixes {
			if s == '*' {
				baseType = GetPointerType(baseType)
			}
		}

		return baseType
	}
}

func AssertType(typ1 llvm.Type, typ2 llvm.Type) bool {
	return typ1.TypeKind() != typ2.TypeKind()
}
