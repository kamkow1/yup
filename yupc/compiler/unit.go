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

type CompilationUnit struct {
	SourceFile string
	ModuleName string
	Builder    llvm.Builder
	Module     llvm.Module
	Locals     []map[string]LocalVariable
	Functions  map[string]Function
}

func NewCompilationUnit(sf string, bc string) *CompilationUnit {
	return &CompilationUnit{
		sf,
		bc,
		llvm.NewBuilder(),
		llvm.NewModule(sf),
		[]map[string]LocalVariable{},
		map[string]Function{},
	}
}

var CompilationUnits = NewStack[CompilationUnit]()

func GetBCWriteData() (llvm.Module, string) {
	mod := CompilationUnits.Peek().Module
	p := CompilationUnits.Peek().ModuleName
	return mod, p
}
