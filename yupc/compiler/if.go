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
	thenBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(*function.Value, "if.then")

	var elseBlock llvm.BasicBlock
	if ctx.IfElseBlock() != nil {
		elseBlock = llvm.AddBasicBlock(*function.Value, "if.else")
	}

	mergeBlock := llvm.AddBasicBlock(*function.Value, "if.merge")

	if ctx.IfElseBlock() != nil {
		CompilationUnits.Peek().Builder.CreateCondBr(cond, thenBlock, elseBlock)
	} else {
		CompilationUnits.Peek().Builder.CreateCondBr(cond, thenBlock, mergeBlock)
	}

	CompilationUnits.Peek().Builder.SetInsertPoint(thenBlock, thenBlock.FirstInstruction())
	hasThenTerminated := v.Visit(ctx.IfThenBlock()).(bool)
	if !hasThenTerminated {
		CompilationUnits.Peek().Builder.CreateBr(mergeBlock)
		CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())
	}

	if ctx.IfElseBlock() != nil {
		CompilationUnits.Peek().Builder.SetInsertPoint(elseBlock, elseBlock.FirstInstruction())
		hasElseTerminated := v.Visit(ctx.IfElseBlock()).(bool)
		if !hasElseTerminated {
			CompilationUnits.Peek().Builder.CreateBr(mergeBlock)
			CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())
		}
	}

	return nil
}
