package compiler

import (
	"tinygo.org/x/go-llvm"
)

type CompilationUnit struct {
	sourceFile string
	moduleName string
	context    *llvm.Context
	builder    *llvm.Builder
	module     *llvm.Module
	valueStack []map[string]*llvm.Value
	typeStack  []map[string]*llvm.Type
	functions  map[string]*llvm.DIFunction
	globals    map[string]*llvm.DIGlobalVariableExpression
}

func NewCompilationUnit(sf string) *CompilationUnit {
	cu := &CompilationUnit{}
	cu.sourceFile = sf
	return cu
}
