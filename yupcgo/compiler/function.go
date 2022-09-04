package compiler

import "tinygo.org/x/go-llvm"

func CreateFuncSignature(name string, paramTypes []llvm.Type, returnType llvm.Type) llvm.Value {
	funcType := llvm.FunctionType(returnType, paramTypes, false)
	function := llvm.AddFunction(compilationUnits.Peek().module, name, funcType)
	compilationUnits.Peek().functions[name] = function

	return function
}

func CreateFunctionEntryBlock(function llvm.Value) {
	block := llvm.AddBasicBlock(function, "entry")
	compilationUnits.Peek().builder.SetInsertPoint(block, block.LastInstruction())

	for _, p := range function.Params() {
		alloca := compilationUnits.Peek().builder.CreateAlloca(p.Type(), "")
		(*compilationUnits.Peek().locals.Peek())[p.Name()] = LocalVariable{
			&Variable{p.Name(), false}, alloca,
		}
		compilationUnits.Peek().builder.CreateStore(p, alloca)
	}
}
