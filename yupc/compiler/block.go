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

	var hasTerminated bool
	for _, st := range ctx.AllStatement() {
		hasReturned := st.(*parser.StatementContext).FunctionReturn() != nil
		hasBranched := st.(*parser.StatementContext).IfStatement() != nil
		hasContinued := st.(*parser.StatementContext).ContinueStatement() != nil
		hasBroken := st.(*parser.StatementContext).BreakStatement() != nil

		hasTerminated = hasReturned || hasBranched || hasContinued || hasBroken
		v.Visit(st)
	}

	RemoveBlock()
	return hasTerminated
}
