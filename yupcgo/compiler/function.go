package compiler

import (
	"fmt"

	"tinygo.org/x/go-llvm"
)

func CreateFuncSignature(name string, paramTypes []llvm.Type, returnType llvm.Type, isExported bool, module *llvm.Module) llvm.Value {
	funcType := llvm.FunctionType(returnType, paramTypes, false)
	function := llvm.AddFunction(*module, name, funcType)
	if isExported || name == "main" {
		function.SetLinkage(llvm.ExternalLinkage)
	} else {
		function.SetLinkage(llvm.PrivateLinkage)
	}

	CompilationUnits.Peek().Functions[name] = function

	return function
}

func CreateFunctionEntryBlock(function llvm.Value, builder *llvm.Builder) {
	block := llvm.AddBasicBlock(function, "entry")
	builder.SetInsertPoint(block, block.LastInstruction())

	if function.ParamsCount() > 0 {
		for _, p := range function.Params() {
			alloca := builder.CreateAlloca(p.Type(), "")
			CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][p.Name()] = LocalVariable{
				&Variable{p.Name(), false}, alloca,
			}
			builder.CreateStore(p, alloca)
		}
	}
}

func CallFunction(name string, args []llvm.Value, builder *llvm.Builder) llvm.Value {
	if f, ok := CompilationUnits.Peek().Functions[name]; ok {
		return builder.CreateCall(f, args, "")
	} else {
		panic(fmt.Sprintf("ERROR: tried to call function %s but it doens't exist", name))
	}
}

func BuildValueReturn(value llvm.Value, builder *llvm.Builder) llvm.Value {
	return builder.CreateRet(value)
}

func BuildVoidReturn(builder *llvm.Builder) llvm.Value {
	return builder.CreateRetVoid()
}
