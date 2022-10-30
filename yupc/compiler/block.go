package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

type BlockExitStatus struct {
	HasReturned  bool
	HasBranched  bool
	HasContinued bool
	HasBrokenOut bool
}

func CreateBlock() {
	m := map[string]LocalVariable{}
	CompilationUnits.Peek().Locals = append(CompilationUnits.Peek().Locals, m)
}

func RemoveBlock() {
	CompilationUnits.Peek().Locals = CompilationUnits.Peek().Locals[:len(CompilationUnits.Peek().Locals)-1]
}

var TrackedAllocsStack = NewStack[Stack[llvm.Value]]()

func TrackAllocation(alloca llvm.Value) {
	TrackedAllocsStack.Peek().Push(&alloca)
}

func (v *AstVisitor) VisitCodeBlock(ctx *parser.CodeBlockContext) any {
	CreateBlock()
	TrackedAllocsStack.Push(NewStack[llvm.Value]())

	var blockExitStatus BlockExitStatus
	for _, st := range ctx.AllStatement() {
		hasReturned := st.(*parser.StatementContext).FuncReturn() != nil
		hasBranched := st.(*parser.StatementContext).IfStatement() != nil
		hasContinued := st.(*parser.StatementContext).ContinueStatement() != nil
		hasBroken := st.(*parser.StatementContext).BreakStatement() != nil

		blockExitStatus = BlockExitStatus{
			HasReturned:  hasReturned,
			HasBranched:  hasBranched,
			HasContinued: hasContinued,
			HasBrokenOut: hasBroken,
		}

		v.Visit(st)
	}

	for _, loc := range CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1] {
		if loc.IsUsed {
			fnname := CompilationUnits.Peek().Builder.GetInsertBlock().Parent().Name()
			LogError("unused variable in function `%s`: %s", fnname, loc.Name)
		}
	}

	llvmLifeTimeEnd := CompilationUnits.Peek().Module.NamedFunction("llvm.lifetime.end.p0i8")
	if llvmLifeTimeEnd.IsNil() {
		pts := []llvm.Type{llvm.Int64Type(), llvm.PointerType(llvm.Int8Type(), 0)}
		ft := llvm.FunctionType(llvm.VoidType(), pts, false)
		llvmLifeTimeEnd = llvm.AddFunction(CompilationUnits.Peek().Module, "llvm.lifetime.end", ft)
	}

	hasTerminated := blockExitStatus.HasBranched || blockExitStatus.HasBrokenOut || blockExitStatus.HasContinued || blockExitStatus.HasReturned
	if !hasTerminated {
		for _, ta := range TrackedAllocsStack.Peek().Units {
			targetData := llvm.NewTargetData(CompilationUnits.Peek().Module.DataLayout())
			size := llvm.ConstInt(llvm.Int64Type(), targetData.TypeAllocSize(ta.Type().ElementType()), false)

			typeinfo := &TypeInfo{
				Type: llvm.PointerType(llvm.Int8Type(), 0),
			}

			args := []llvm.Value{size, Cast(*ta, typeinfo)}
			_ = args

			if blockExitStatus.HasReturned {
				CompilationUnits.Peek().Builder.CreateCall(llvm.VoidType(), llvmLifeTimeEnd, args, "")
			}
		}
	}

	RemoveBlock()
	TrackedAllocsStack.Pop()
	return blockExitStatus
}
