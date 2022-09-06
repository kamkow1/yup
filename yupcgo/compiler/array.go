package compiler

import "tinygo.org/x/go-llvm"

func CreateArray(vals []llvm.Value) llvm.Value {
	typ := vals[0].Type()
	return llvm.ConstArray(typ, vals)
}
