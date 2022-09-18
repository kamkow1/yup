package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
)

func CreateBlock() {
	m := map[string]LocalVariable{}
	CompilationUnits.Peek().Locals = append(CompilationUnits.Peek().Locals, m)
}

func RemoveBlock() {
	CompilationUnits.Peek().Locals = CompilationUnits.Peek().Locals[:len(CompilationUnits.Peek().Locals)-1]
}

func (v *AstVisitor) VisitCodeBlock(ctx *parser.CodeBlockContext) any {
	CreateBlock()

	var isRet bool
	for _, st := range ctx.AllStatement() {
		isRet = st.(*parser.StatementContext).FunctionReturn() != nil
		v.Visit(st)
	}

	RemoveBlock()
	return isRet
}
