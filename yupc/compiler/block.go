package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
)

func CreateBlock() {
	m := map[string]LocalVariable{}
	CompilationUnits.Peek().Locals = append(CompilationUnits.Peek().Locals, m)
}

func RemoveBlock() []map[string]LocalVariable {
	return RemoveIndex(CompilationUnits.Peek().Locals, len(CompilationUnits.Peek().Locals)-1)
}

func (v *AstVisitor) VisitCodeBlock(ctx *parser.CodeBlockContext) any {
	CreateBlock()
	for _, st := range ctx.AllStatement() {
		v.Visit(st)
	}

	return RemoveBlock()
}
