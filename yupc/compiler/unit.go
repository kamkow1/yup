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
	Globals    map[string]llvm.Value
	Functions  map[string]Function
	Structs    map[string]Structure
	Types 	   map[string]llvm.Type
}

func NewCompilationUnit(sf string, bc string) *CompilationUnit {
	mod := llvm.NewModule(sf)
	target, _ := llvm.GetTargetFromTriple(GlobalCompilerInfo.TargetTriple)


	machine := target.CreateTargetMachine(
		GlobalCompilerInfo.TargetTriple, 
		"generic", 
		"",
		llvm.CodeGenLevelAggressive,
		llvm.RelocDefault,
		llvm.CodeModelDefault,
	)
	
	mod.SetDataLayout(machine.CreateTargetData().String())
	mod.SetTarget(GlobalCompilerInfo.TargetTriple)
	
	return &CompilationUnit{
		SourceFile: sf,
		ModuleName: bc,
		Builder:    llvm.NewBuilder(),
		Module: 	mod,
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
