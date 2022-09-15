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

	functionName := CompilationUnits.Peek().Builder.GetInsertBlock().Parent().Name()
	function := CompilationUnits.Peek().Functions[functionName]
	thenBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(function.LLVMValue, "")

	var elseBlock llvm.BasicBlock
	if ctx.IfElseBlock() != nil {
		elseBlock = llvm.AddBasicBlock(function.LLVMValue, "")
	}

	mergeBlock := llvm.AddBasicBlock(function.LLVMValue, "")

	if ctx.IfElseBlock() != nil {
		CompilationUnits.Peek().Builder.CreateCondBr(cond, thenBlock, elseBlock)
	} else {
		CompilationUnits.Peek().Builder.CreateCondBr(cond, thenBlock, mergeBlock)
	}

	CompilationUnits.Peek().Builder.SetInsertPoint(thenBlock, thenBlock.FirstInstruction())
	v.Visit(ctx.IfThenBlock())
	CompilationUnits.Peek().Builder.CreateBr(*function.ExitBlock)

	if ctx.IfElseBlock() != nil {
		CompilationUnits.Peek().Builder.SetInsertPoint(elseBlock, elseBlock.FirstInstruction())
		v.Visit(ctx.IfElseBlock())
		CompilationUnits.Peek().Builder.CreateBr(*function.ExitBlock)
	}

	return nil
}
