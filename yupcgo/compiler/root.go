package compiler

type CompilerOptions struct {
	sourcePaths []string
	outputPath  string
}

func (co *CompilerOptions) AddSourcePath(p string) {
	co.sourcePaths = append(co.sourcePaths, p)
}

func (co *CompilerOptions) SetOutputPath(p string) {
	co.outputPath = p
}

var compilerOptions = &CompilerOptions{}
