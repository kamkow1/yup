package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitIfThenBlock(ctx *parser.IfThenBlockContext) any {
	return v.Visit(ctx.CodeBlock())
}

func (v *AstVisitor) VisitIfElseBlock(ctx *parser.IfElseBlockContext) any {
	return v.Visit(ctx.CodeBlock())
}

func (v *AstVisitor) VisitIfStatement(ctx *parser.IfStatementContext) any {
	cond := v.Visit(ctx.Expression()).(llvm.Value)
	//	function := CompilationUnits.Peek().Builder.GetInsertBlock().Parent()
	thenBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(
		CompilationUnits.Peek().Builder.GetInsertBlock().Parent(), "")

	elseBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(
		CompilationUnits.Peek().Builder.GetInsertBlock().Parent(), "")

	mergeBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(
		CompilationUnits.Peek().Builder.GetInsertBlock().Parent(), "")

	br := CompilationUnits.Peek().Builder.CreateCondBr(cond, thenBlock, elseBlock)

	CompilationUnits.Peek().Builder.SetInsertPoint(thenBlock, thenBlock.LastInstruction())
	v.Visit(ctx.IfThenBlock())

	CompilationUnits.Peek().Builder.CreateBr(mergeBlock)
	//thenBlock = CompilationUnits.Peek().Builder.GetInsertBlock()

	CompilationUnits.Peek().Builder.SetInsertPoint(elseBlock, elseBlock.LastInstruction())
	v.Visit(ctx.IfElseBlock())

	CompilationUnits.Peek().Builder.CreateBr(mergeBlock)
	//elseBlock = CompilationUnits.Peek().Builder.GetInsertBlock()

	//phi := CompilationUnits.Peek().Builder.CreatePHI(llvm.DoubleType(), "")
	//phi.AddIncoming()

	return br
}
