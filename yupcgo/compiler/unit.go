package compiler

import (
	"tinygo.org/x/go-llvm"
)

type Stack[T any] struct {
	units []*T
}

func NewStack[T any]() *Stack[T] {
	return &Stack[T]{}
}

func (s *Stack[T]) Push(item *T) {
	s.units = append(s.units, item)
}

func (s *Stack[T]) Pop() *T {
	cu := s.units[len(s.units)-1]
	s.units = s.units[0 : len(s.units)-1]
	return cu
}

func (s *Stack[T]) Peek() *T {
	return s.units[len(s.units)-1]
}

func RemoveIndex[T any](s []T, index int) []T {
	ret := make([]T, 0)
	ret = append(ret, s[:index]...)
	return append(ret, s[index+1:]...)
}

type CompilationUnit struct {
	sourceFile string
	moduleName string
	context    llvm.Context
	builder    llvm.Builder
	module     llvm.Module
	valueStack Stack[[]map[string]llvm.Value]
	typeStack  Stack[[]map[string]llvm.Type]
	functions  map[string]llvm.Value
	globals    map[string]*GlobalVariable
	locals     []map[string]LocalVariable
}

func NewCompilationUnit(sf string, bc string) *CompilationUnit {
	return &CompilationUnit{
		sf,
		bc,
		llvm.NewContext(),
		llvm.NewBuilder(),
		llvm.NewModule(sf),
		Stack[[]map[string]llvm.Value]{},
		Stack[[]map[string]llvm.Type]{},
		map[string]llvm.Value{},
		map[string]*GlobalVariable{},
		[]map[string]LocalVariable{},
	}
}

func GetBCWriteData() (llvm.Module, string) {
	mod := compilationUnits.Peek().module
	p := compilationUnits.Peek().moduleName
	return mod, p
}

func DebugPrintModule() {
	compilationUnits.Peek().module.Dump()
}
