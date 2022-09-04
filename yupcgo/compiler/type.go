package compiler

import "tinygo.org/x/go-llvm"

var builtinLLVMTypes map[string]llvm.Type

func init() {
	builtinLLVMTypes = map[string]llvm.Type{
		"i32": llvm.Int32Type(),
	}
}

func GetTypeFromName(name string) llvm.Type {
	if llvmType, ok := builtinLLVMTypes[name]; ok {
		return llvmType
	}

	panic("unknown type")
}
