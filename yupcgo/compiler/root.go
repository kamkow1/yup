package compiler

type CompilerOptions struct {
	sourcePaths []string
	outputPath  string
}

var compilerOptions = &CompilerOptions{}
var compilationUnits = NewStack[CompilationUnit]()

func GetCompilationUnits() *Stack[CompilationUnit] {
	return compilationUnits
}
