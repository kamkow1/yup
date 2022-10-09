package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func CreateBlock() {
	m := map[string]LocalVariable{}
	CompilationUnits.Peek().Locals = append(CompilationUnits.Peek().Locals, m)
}

func RemoveBlock() {
	CompilationUnits.Peek().Locals = CompilationUnits.Peek().Locals[:len(CompilationUnits.Peek().Locals)-1]
}

var TrackedAllocations []llvm.Value = make([]llvm.Value, 0)

func TrackAllocation(alloca llvm.Value) {
	TrackedAllocations = append(TrackedAllocations, alloca)
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

	llvmLifeTimeEnd := CompilationUnits.Peek().Module.NamedFunction("llvm.lifetime.end")
	if llvmLifeTimeEnd.IsNil() {
		pts := []llvm.Type{llvm.Int64Type(), llvm.PointerType(llvm.Int8Type(), 0)}
		ft := llvm.FunctionType(llvm.VoidType(), pts, false)
		llvmLifeTimeEnd = llvm.AddFunction(CompilationUnits.Peek().Module, "llvm.lifetime.end", ft)
	}

	for _, ta := range TrackedAllocations {
		targetData := llvm.NewTargetData(CompilationUnits.Peek().Module.DataLayout())
		size := llvm.ConstInt(llvm.Int64Type(), targetData.TypeAllocSize(ta.Type().ElementType()), false)
		args := []llvm.Value{size, Cast(ta, llvm.PointerType(llvm.Int8Type(), 0))}
		CompilationUnits.Peek().Builder.CreateCall(llvmLifeTimeEnd, args, "")
	}

	RemoveBlock()
	return hasTerminated
}
