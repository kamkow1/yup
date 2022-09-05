package compiler

import (
	"fmt"

	"tinygo.org/x/go-llvm"
)

func CreateFuncSignature(name string, paramTypes []llvm.Type, returnType llvm.Type, isExported bool) llvm.Value {
	funcType := llvm.FunctionType(returnType, paramTypes, false)
	function := llvm.AddFunction(CompilationUnits.Peek().Module, name, funcType)
	if isExported || name == "main" {
		function.SetLinkage(llvm.ExternalLinkage)
	} else {
		function.SetLinkage(llvm.PrivateLinkage)
	}

	CompilationUnits.Peek().Functions[name] = function

	return function
}

func CreateFunctionEntryBlock(function llvm.Value) {
	block := llvm.AddBasicBlock(function, "entry")
	CompilationUnits.Peek().Builder.SetInsertPoint(block, block.LastInstruction())

	if function.ParamsCount() > 0 {
		for _, p := range function.Params() {
			alloca := CompilationUnits.Peek().Builder.CreateAlloca(p.Type(), "")
			CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][p.Name()] = LocalVariable{
				&Variable{p.Name(), false}, alloca,
			}
			CompilationUnits.Peek().Builder.CreateStore(p, alloca)
		}
	}
}

func CallFunction(name string, args []llvm.Value) llvm.Value {
	if f, ok := CompilationUnits.Peek().Functions[name]; ok {
		return CompilationUnits.Peek().Builder.CreateCall(f, args, "")
	} else {
		panic(fmt.Sprintf("ERROR: tried to call function %s but it doens't exist", name))
	}
}

func BuildValueReturn(value llvm.Value) llvm.Value {
	return CompilationUnits.Peek().Builder.CreateRet(value)
}

func BuildVoidReturn() llvm.Value {
	return CompilationUnits.Peek().Builder.CreateRetVoid()
}
