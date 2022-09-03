package compiler

import (
	"github.com/kamkow1/yup/yupcgo/ast"
	"github.com/kamkow1/yup/yupcgo/lexer"
	"github.com/kamkow1/yup/yupcgo/parser"

	"github.com/antlr/antlr4/runtime/Go/antlr"
)

type Compiler struct {
	options *CompilerOptions
}

type CompilerOptions struct {
	sourcePaths []string
	outputPath  string
}

var compilerOptions = &CompilerOptions{}

func NewCompiler() *Compiler {
	return &Compiler{}
}

func (c *Compiler) ProcessSourceFile(file string) {
	is := antlr.NewInputStream(file)
	lexer := lexer.NewYupLexer(is)
	tokens := antlr.NewCommonTokenStream(lexer, antlr.TokenDefaultChannel)
	parser := parser.NewYupParser(tokens)

	parser.BuildParseTrees = true
	tree := parser.File()
	v := ast.NewAstVisitor()

	v.Visit(tree)
}
