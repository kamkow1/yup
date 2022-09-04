package compiler

import (
	"fmt"

	"tinygo.org/x/go-llvm"
)

func CreateFuncSignature(name string, paramTypes []llvm.Type, returnType llvm.Type, isExported bool) llvm.Value {
	funcType := llvm.FunctionType(returnType, paramTypes, false)
	function := llvm.AddFunction(compilationUnits.Peek().module, name, funcType)
	if isExported || name == "main" {
		function.SetLinkage(llvm.ExternalLinkage)
	} else {
		function.SetLinkage(llvm.PrivateLinkage)
	}

	compilationUnits.Peek().functions[name] = function

	return function
}

func CreateFunctionEntryBlock(function llvm.Value) {
	block := llvm.AddBasicBlock(function, "entry")
	compilationUnits.Peek().builder.SetInsertPoint(block, block.LastInstruction())

	if function.ParamsCount() > 0 {
		for _, p := range function.Params() {
			alloca := compilationUnits.Peek().builder.CreateAlloca(p.Type(), "")
			compilationUnits.Peek().locals[len(compilationUnits.Peek().locals)-1][p.Name()] = LocalVariable{
				&Variable{p.Name(), false}, alloca,
			}
			compilationUnits.Peek().builder.CreateStore(p, alloca)
		}
	}
}

func CallFunction(name string, args []llvm.Value) llvm.Value {
	if f, ok := compilationUnits.Peek().functions[name]; ok {
		return compilationUnits.Peek().builder.CreateCall(f, args, "")
	} else {
		panic(fmt.Sprintf("ERROR: tried to call function %s but it doens't exist", name))
	}
}

func BuildValueReturn(value llvm.Value) llvm.Value {
	return compilationUnits.Peek().builder.CreateRet(value)
}

func BuildVoidReturn() llvm.Value {
	return compilationUnits.Peek().builder.CreateRetVoid()
}
