package compiler

import (
	"fmt"

	"tinygo.org/x/go-llvm"
)

type functionType func([]string) llvm.Value

var functions map[string]functionType = map[string]functionType{
	"Cast": Cast,
}

func Cast(args []string) llvm.Value {
	value := FindLocalVariable(args[0], len(CompilationUnits.Peek().Locals)-1).Value
	load := CompilationUnits.Peek().Builder.CreateLoad(value, "")
	typ, ok := BuiltinLLVMTypes[args[1]]
	if !ok {
		panic(fmt.Sprintf("ERROR: unknown type: %s", args[1]))
	}

	if load.Type().TypeKind() == llvm.IntegerTypeKind && typ.TypeKind() == llvm.IntegerTypeKind {
		return CompilationUnits.Peek().Builder.CreateIntCast(load, typ, "")
	} else if load.Type().TypeKind() == llvm.IntegerTypeKind && typ.TypeKind() == llvm.PointerTypeKind {
		return CompilationUnits.Peek().Builder.CreateIntToPtr(load, typ, "")
	} else if load.Type().TypeKind() == llvm.PointerTypeKind && typ.TypeKind() == llvm.IntegerTypeKind {
		return CompilationUnits.Peek().Builder.CreatePtrToInt(load, typ, "")
	} else if load.Type().TypeKind() == llvm.PointerTypeKind && typ.TypeKind() == llvm.PointerTypeKind {
		return CompilationUnits.Peek().Builder.CreatePointerCast(load, typ, "")
	} else {
		return CompilationUnits.Peek().Builder.CreateBitCast(load, typ, "")
	}
}

func CallBuiltinFunction(name string, args []string) llvm.Value {
	return functions[name](args)
}
