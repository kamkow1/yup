package compiler

import (
	"log"
	"strconv"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func (v *AstVisitor) VisitRangeExpression(ctx *parser.RangeExpressionContext) any {
	bound0, err0 := strconv.ParseInt(ctx.ValueInteger(0).GetText(), 10, 64)
	bound1, err1 := strconv.ParseInt(ctx.ValueInteger(1).GetText(), 10, 64)

	if err0 != nil {
		log.Fatalf("ERROR: failed to parse lower bound of range expr: %v", err0)
	}

	if err1 != nil {
		log.Fatalf("ERROR: failed to parse upper bound of range expr: %v", err1)
	}

	var elems []llvm.Value
	for i := bound0; i <= bound1; i++ {
		val := llvm.ConstInt(llvm.Int64Type(), uint64(i), false)
		elems = append(elems, val)
	}

	return llvm.ConstArray(llvm.Int64Type(), elems)
}

func (v *AstVisitor) VisitConditionBasedLoop(ctx *parser.ConditionBasedLoopContext) any {
	return v.Visit(ctx.Expression())
}

type Loop struct {
	bodyBlock       llvm.BasicBlock
	mergeBlock      llvm.BasicBlock
	skipCurrentIter bool
	breakLoop       bool
}

var LoopStack Stack[Loop] = *NewStack[Loop]()

func (v *AstVisitor) VisitForLoopStatement(ctx *parser.ForLoopStatementContext) any {
	if ctx.ConditionBasedLoop() != nil {
		cond0 := v.Visit(ctx.ConditionBasedLoop()).(llvm.Value)
		condValue := CompilationUnits.Peek().Builder.CreateAlloca(cond0.Type(), "cond_value")
		CompilationUnits.Peek().Builder.CreateStore(cond0, condValue)

		functionName := CompilationUnits.Peek().Builder.GetInsertBlock().Parent().Name()
		function := CompilationUnits.Peek().Functions[functionName]

		loopBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(function.Value, "for.body")
		mergeBlock := CompilationUnits.Peek().Module.Context().AddBasicBlock(function.Value, "for.merge")
		LoopStack.Push(&Loop{loopBlock, mergeBlock, false, false})

		boolFalse := llvm.ConstInt(llvm.Int1Type(), uint64(0), false)
		load0 := CompilationUnits.Peek().Builder.CreateLoad(condValue, "")
		cmp := CompilationUnits.Peek().Builder.CreateICmp(llvm.IntPredicate(llvm.IntNE), load0, boolFalse, "")
		CompilationUnits.Peek().Builder.CreateCondBr(cmp, loopBlock, mergeBlock)

		CompilationUnits.Peek().Builder.SetInsertPoint(loopBlock, loopBlock.FirstInstruction())

		hasTerminated := v.Visit(ctx.CodeBlock()).(bool)
		if !hasTerminated {
			cond1 := v.Visit(ctx.ConditionBasedLoop()).(llvm.Value)
			CompilationUnits.Peek().Builder.CreateStore(cond1, condValue)

			load1 := CompilationUnits.Peek().Builder.CreateLoad(condValue, "")
			CompilationUnits.Peek().Builder.CreateCondBr(load1, loopBlock, mergeBlock)
			CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())
		} else if LoopStack.Peek().skipCurrentIter {
			CompilationUnits.Peek().Builder.SetInsertPoint(loopBlock, loopBlock.FirstInstruction())
		} else if LoopStack.Peek().breakLoop {
			CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())
		} else {
			CompilationUnits.Peek().Builder.CreateBr(mergeBlock)
			CompilationUnits.Peek().Builder.SetInsertPoint(mergeBlock, mergeBlock.FirstInstruction())
		}
	}

	return nil
}

func (v *AstVisitor) VisitContinueStatement(ctx *parser.ContinueStatementContext) any {
	loop := LoopStack.Peek().bodyBlock
	LoopStack.Peek().skipCurrentIter = true
	return CompilationUnits.Peek().Builder.CreateBr(loop)
}

func (v *AstVisitor) VisitBreakStatement(ctx *parser.BreakStatementContext) any {
	merge := LoopStack.Peek().mergeBlock
	LoopStack.Peek().breakLoop = true
	return CompilationUnits.Peek().Builder.CreateBr(merge)
}
