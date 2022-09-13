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

	thenBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(
		CompilationUnits.Peek().Builder.GetInsertBlock().Parent(), "")

	if ctx.IfElseBlock() != nil {
		elseBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(
			CompilationUnits.Peek().Builder.GetInsertBlock().Parent(), "")

		_ = CompilationUnits.Peek().Builder.CreateCondBr(cond, thenBlock, elseBlock)

		CompilationUnits.Peek().Builder.SetInsertPointAtEnd(thenBlock)
		v.Visit(ctx.IfThenBlock())

		mergeBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(
			CompilationUnits.Peek().Builder.GetInsertBlock().Parent(), "")
		CompilationUnits.Peek().Builder.CreateBr(mergeBlock)

		CompilationUnits.Peek().Builder.SetInsertPointAtEnd(elseBlock)
		v.Visit(ctx.IfElseBlock())
		CompilationUnits.Peek().Builder.CreateBr(mergeBlock)
	} else {
		//dummyElseBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(
		//	CompilationUnits.Peek().Builder.GetInsertBlock().Parent(), "")

		mergeBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(
			CompilationUnits.Peek().Builder.GetInsertBlock().Parent(), "")

		_ = CompilationUnits.Peek().Builder.CreateCondBr(cond, thenBlock, mergeBlock)

		CompilationUnits.Peek().Builder.SetInsertPoint(thenBlock, thenBlock.LastInstruction())
		v.Visit(ctx.IfThenBlock())
		CompilationUnits.Peek().Builder.CreateBr(mergeBlock)
	}

	return nil
}
