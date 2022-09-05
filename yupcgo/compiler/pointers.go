package compiler

import "tinygo.org/x/go-llvm"

func DereferencePointer(ptr llvm.Value, builder *llvm.Builder) llvm.Value {
	return builder.CreateLoad(ptr, "")
}
