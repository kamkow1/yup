package compiler

import (
	"tinygo.org/x/go-llvm"
)

type Stack[T any] struct {
	Units []*T
}

func NewStack[T any]() *Stack[T] {
	return &Stack[T]{}
}

func (s *Stack[T]) Push(item *T) {
	s.Units = append(s.Units, item)
}

func (s *Stack[T]) Pop() *T {
	cu := s.Units[len(s.Units)-1]
	s.Units = s.Units[0 : len(s.Units)-1]
	return cu
}

func (s *Stack[T]) Peek() *T {
	return s.Units[len(s.Units)-1]
}

type CompilationUnit struct {
	SourceFile string
	ModuleName string
	Builder    llvm.Builder
	Module     llvm.Module
	Locals     []map[string]LocalVariable
	Globals    map[string]llvm.Value
	Functions  map[string]Function
	Structs    map[string]Structure
	Types 	   map[string]llvm.Type
}

func NewCompilationUnit(sf string, bc string) *CompilationUnit {
	/*target, _ := llvm.GetTargetFromTriple(GlobalCompilerInfo.TargetTriple)


	machine := target.CreateTargetMachine(
		GlobalCompilerInfo.TargetTriple, 
		"generic", 
		"",
		llvm.CodeGenLevelNone,
		llvm.RelocDefault,
		llvm.CodeModelDefault,
	)
	
	mod.SetDataLayout(machine.CreateTargetData().String())
	mod.SetTarget(GlobalCompilerInfo.TargetTriple)*/
	
	return &CompilationUnit{
		SourceFile: sf,
		ModuleName: bc,
		Builder:    llvm.NewBuilder(),
		Module:		llvm.NewModule(sf),
		Locals:     []map[string]LocalVariable{},
		Globals:    map[string]llvm.Value{},
		Functions:  map[string]Function{},
		Structs:    map[string]Structure{},
		Types:		InitTypeMap(),
	}
}

var CompilationUnits = NewStack[CompilationUnit]()

func GetBCWriteData() (llvm.Module, string) {
	mod := CompilationUnits.Peek().Module
	p := CompilationUnits.Peek().ModuleName
	return mod, p
}
