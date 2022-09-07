package compiler

import (
	"fmt"

	"tinygo.org/x/go-llvm"
)

type functionType func([]string) llvm.Value

var functions map[string]functionType = map[string]functionType{
	"intCast":  intCast,
	"bitCast":  bitCast,
	"ptrToInt": ptrToIntCast,
}

func intCast(args []string) llvm.Value {
	value := FindLocalVariable(args[0], len(CompilationUnits.Peek().Locals)-1).Value
	load := CompilationUnits.Peek().Builder.CreateLoad(value, "")
	typ, ok := BuiltinLLVMTypes[args[1]]
	if !ok {
		panic(fmt.Sprintf("ERROR: unknown type: %s", args[1]))
	}

	return CompilationUnits.Peek().Builder.CreateIntCast(load, typ, "")
}

func bitCast(args []string) llvm.Value {
	value := FindLocalVariable(args[0], len(CompilationUnits.Peek().Locals)-1).Value
	load := CompilationUnits.Peek().Builder.CreateLoad(value, "")
	typ, ok := BuiltinLLVMTypes[args[1]]
	if !ok {
		panic(fmt.Sprintf("ERROR: unknown type: %s", args[1]))
	}

	return CompilationUnits.Peek().Builder.CreateBitCast(load, typ, "")
}

func ptrToIntCast(args []string) llvm.Value {
	value := FindLocalVariable(args[0], len(CompilationUnits.Peek().Locals)-1).Value
	load := CompilationUnits.Peek().Builder.CreateLoad(value, "")
	typ, ok := BuiltinLLVMTypes[args[1]]
	if !ok {
		panic(fmt.Sprintf("ERROR: unknown type: %s", args[1]))
	}

	fmt.Println(value.Type().String())
	fmt.Println(typ.String())
	return CompilationUnits.Peek().Builder.CreatePtrToInt(load, typ, "")
}

func CallBuiltinFunction(name string, args []string) llvm.Value {
	return functions[name](args)
}
