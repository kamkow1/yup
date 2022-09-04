package compiler

var compilationUnits = NewStack[CompilationUnit]()

func GetCompilationUnits() *Stack[CompilationUnit] {
	return compilationUnits
}
