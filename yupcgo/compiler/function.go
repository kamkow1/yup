package compiler

import (
	"tinygo.org/x/go-llvm"
)

type FuncParam struct {
	Name string
	Type llvm.Type
}

func CreateFuncSignature(name string, params []FuncParam,
	returnType llvm.Type, module *llvm.Module) llvm.Value {
	var paramTypes []llvm.Type
	for _, fp := range params {
		paramTypes = append(paramTypes, fp.Type)
	}

	funcType := llvm.FunctionType(returnType, paramTypes, false)
	function := llvm.AddFunction(*module, name, funcType)

	return function
}

func CreateFunctionEntryBlock(function llvm.Value, builder *llvm.Builder) {
	block := llvm.AddBasicBlock(function, "entry")
	builder.SetInsertPoint(block, block.LastInstruction())

	if function.ParamsCount() > 0 {
		for _, p := range function.Params() {
			alloca := builder.CreateAlloca(p.Type(), "")
			n := len(CompilationUnits.Peek().Locals) - 1
			CompilationUnits.Peek().Locals[n][p.Name()] = LocalVariable{p.Name(), false, alloca}
			builder.CreateStore(p, alloca)
		}
	}
}

func CallFunction(name string, args []llvm.Value, builder *llvm.Builder) llvm.Value {
	f := CompilationUnits.Peek().Module.NamedFunction(name)
	return builder.CreateCall(f, args, "")
	/*if f, ok := CompilationUnits.Peek().Functions[name]; ok {
		return builder.CreateCall(f, args, "")
	} else {
		panic(fmt.Sprintf("ERROR: tried to call function %s but it doens't exist", name))
	}*/
}

func BuildValueReturn(value llvm.Value, builder *llvm.Builder) llvm.Value {
	return builder.CreateRet(value)
}

func BuildVoidReturn(builder *llvm.Builder) llvm.Value {
	return builder.CreateRetVoid()
}
