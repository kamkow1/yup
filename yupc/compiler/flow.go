package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitConditionBasedLoop(ctx *parser.ConditionBasedLoopContext) any {
	return v.Visit(ctx.Expression())
}

func (v *AstVisitor) VisitStatementBasedLoop(ctx *parser.StatementBasedLoopContext) any {
	return nil
}

type Loop struct {
	BodyBlock       llvm.BasicBlock
	MergeBlock      llvm.BasicBlock
	SkipCurrentIter bool
	BreakLoop       bool
	FinalStatement  *parser.StatementContext
}

var LoopStack Stack[Loop] = *NewStack[Loop]()

func ResetAfterBranching() {
	LoopStack.Peek().BreakLoop = false
	LoopStack.Peek().SkipCurrentIter = false
}

func (v *AstVisitor) VisitForLoopStatement(ctx *parser.ForLoopStatementContext) any {
	functionName := CompilationUnits.Peek().Builder.GetInsertBlock().Parent().Name()
	function := CompilationUnits.Peek().Functions[functionName]

	loopBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(*function.Value, "for.body")
	mergeBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(*function.Value, "for.merge")

	prepBlock := llvm.AddBasicBlock(*function.Value, "for.prep")
	CompilationUnits.Peek().Builder.CreateBr(prepBlock)
	CompilationUnits.Peek().Builder.SetInsertPoint(prepBlock, prepBlock.FirstInstruction())

	LoopStack.Push(&Loop{
		loopBlock,
		mergeBlock,
		false,
		false,
		nil,
	})

	if ctx.ConditionBasedLoop() != nil {
		cond0 := v.Visit(ctx.ConditionBasedLoop()).(llvm.Value)
		i1typeinfo := &TypeInfo{
			Type: llvm.Int1Type(),	
		}
		if cond0.Type() != llvm.Int1Type() {	
			cond0 = Cast(cond0, i1typeinfo)
		}
		
		condValue := CompilationUnits.Peek().Builder.CreateAlloca(cond0.Type(), "cond_value")
		CompilationUnits.Peek().Builder.CreateStore(cond0, condValue)

		boolFalse := llvm.ConstInt(llvm.Int1Type(), uint64(0), false)
		load0 := CompilationUnits.Peek().Builder.CreateLoad(condValue.AllocatedType(), condValue, "")
		cmp := CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.IntNE), load0, boolFalse, "")
		CompilationUnits.Peek().Builder.CreateCondBr(cmp, loopBlock, mergeBlock)

		CompilationUnits.Peek().Builder.SetInsertPoint(loopBlock, loopBlock.FirstInstruction())

		exitStatus := v.Visit(ctx.CodeBlock()).(BlockExitStatus)
		hasTerminated := exitStatus.HasReturned || exitStatus.HasContinued || exitStatus.HasBrokenOut || exitStatus.HasBranched
		if !hasTerminated {
			cond1 := v.Visit(ctx.ConditionBasedLoop()).(llvm.Value)
			if cond1.Type() != llvm.Int1Type() {
				cond1 = Cast(cond1, i1typeinfo)
			}
			
			CompilationUnits.Peek().Builder.CreateStore(cond1, condValue)

			load1 := CompilationUnits.Peek().Builder.CreateLoad(condValue.AllocatedType(), condValue, "")
			CompilationUnits.Peek().Builder.CreateCondBr(load1, loopBlock, mergeBlock)
			CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())
		} else if LoopStack.Peek().SkipCurrentIter {
			CompilationUnits.Peek().Builder.SetInsertPoint(loopBlock, loopBlock.FirstInstruction())
			ResetAfterBranching()
		} else if LoopStack.Peek().BreakLoop {
			CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())
			ResetAfterBranching()
		} else {
			CompilationUnits.Peek().Builder.CreateBr(mergeBlock)
			CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())
		}
	} else if ctx.StatementBasedLoop() != nil {
		CreateBlock()
		sblctx := ctx.StatementBasedLoop().(*parser.StatementBasedLoopContext)
		v.Visit(sblctx.Statement(0))
		LoopStack.Peek().FinalStatement = sblctx.Statement(2).(*parser.StatementContext)

		cond0 := v.Visit(sblctx.Statement(1)).(llvm.Value)
		condValue := CompilationUnits.Peek().Builder.CreateAlloca(cond0.Type(), "cond_value")
		CompilationUnits.Peek().Builder.CreateStore(cond0, condValue)

		boolFalse := llvm.ConstInt(llvm.Int1Type(), uint64(0), false)
		load0 := CompilationUnits.Peek().Builder.CreateLoad(condValue.AllocatedType(), condValue, "")
		cmp := CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.IntNE), load0, boolFalse, "")

		CompilationUnits.Peek().Builder.CreateCondBr(cmp, loopBlock, mergeBlock)
		CompilationUnits.Peek().Builder.SetInsertPoint(loopBlock, loopBlock.FirstInstruction())

		exitStatus := v.Visit(ctx.CodeBlock()).(BlockExitStatus)
		hasTerminated := exitStatus.HasReturned || exitStatus.HasBranched || exitStatus.HasContinued || exitStatus.HasBrokenOut
		if !hasTerminated {
			v.Visit(LoopStack.Peek().FinalStatement)
			cond1 := v.Visit(sblctx.Statement(1)).(llvm.Value)
			CompilationUnits.Peek().Builder.CreateStore(cond1, condValue)

			load1 := CompilationUnits.Peek().Builder.CreateLoad(condValue.AllocatedType(), condValue, "")
			CompilationUnits.Peek().Builder.CreateCondBr(load1, loopBlock, mergeBlock)
			CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())
		} else if LoopStack.Peek().SkipCurrentIter {
			CompilationUnits.Peek().Builder.SetInsertPoint(loopBlock, loopBlock.FirstInstruction())
			ResetAfterBranching()
		} else if LoopStack.Peek().BreakLoop {
			CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())
			ResetAfterBranching()
		} else {
			CompilationUnits.Peek().Builder.CreateBr(mergeBlock)
			CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())
		}

		RemoveBlock()
	}

	return nil
}

func (v *AstVisitor) VisitContinueStatement(ctx *parser.ContinueStatementContext) any {
	LoopStack.Peek().SkipCurrentIter = true
	loop := LoopStack.Peek().BodyBlock

	if LoopStack.Peek().FinalStatement != nil {
		v.Visit(LoopStack.Peek().FinalStatement)
	}

	return CompilationUnits.Peek().Builder.CreateBr(loop)
}

func (v *AstVisitor) VisitBreakStatement(ctx *parser.BreakStatementContext) any {
	merge := LoopStack.Peek().MergeBlock
	LoopStack.Peek().BreakLoop = true
	return CompilationUnits.Peek().Builder.CreateBr(merge)
}

func (v *AstVisitor) VisitIfThenBlock(ctx *parser.IfThenBlockContext) any {
	return v.Visit(ctx.CodeBlock())
}

func (v *AstVisitor) VisitIfElseBlock(ctx *parser.IfElseBlockContext) any {
	return v.Visit(ctx.CodeBlock())
}

func (v *AstVisitor) VisitIfStatement(ctx *parser.IfStatementContext) any {
	cond := v.Visit(ctx.Expression()).(llvm.Value)
	if cond.Type() != llvm.Int1Type() {
		i1typeinfo := &TypeInfo{
			Type: llvm.Int1Type(),
		}
		
		cond = Cast(cond, i1typeinfo)
	}

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
	exitStatus := v.Visit(ctx.IfThenBlock()).(BlockExitStatus)
	if !exitStatus.HasReturned {
		CompilationUnits.Peek().Builder.CreateBr(mergeBlock)
	}

	CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())

	if ctx.IfElseBlock() != nil {
		CompilationUnits.Peek().Builder.SetInsertPoint(elseBlock, elseBlock.FirstInstruction())
		exitStatus := v.Visit(ctx.IfElseBlock()).(BlockExitStatus)
		hasElseTerminated := exitStatus.HasBranched || exitStatus.HasReturned || exitStatus.HasContinued || exitStatus.HasBrokenOut
		if !hasElseTerminated {
			CompilationUnits.Peek().Builder.CreateBr(mergeBlock)
			CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())
		}
	}

	return nil
}