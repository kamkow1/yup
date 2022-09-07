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
	SourceFile string
	ModuleName string
	Builder    llvm.Builder
	Module     llvm.Module
	ValueStack Stack[[]map[string]llvm.Value]
	TypeStack  Stack[[]map[string]llvm.Type]
	Locals     []map[string]LocalVariable
}

func NewCompilationUnit(sf string, bc string) *CompilationUnit {
	return &CompilationUnit{
		sf,
		bc,
		llvm.NewBuilder(),
		llvm.NewModule(sf),
		Stack[[]map[string]llvm.Value]{},
		Stack[[]map[string]llvm.Type]{},
		[]map[string]LocalVariable{},
	}
}

func GetBCWriteData() (llvm.Module, string) {
	mod := CompilationUnits.Peek().Module
	p := CompilationUnits.Peek().ModuleName
	return mod, p
}

func DebugPrintModule() {
	CompilationUnits.Peek().Module.Dump()
}
