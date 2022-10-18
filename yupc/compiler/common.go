package compiler

import (
	"tinygo.org/x/go-llvm"
)

func CreateAllocation(typ llvm.Type) llvm.Value {
	alloca := CompilationUnits.Peek().Builder.CreateAlloca(typ, "")
	TrackAllocation(alloca)

	ltsname := "llvm.lifetime.start"
	lifetimeStart := CompilationUnits.Peek().Module.NamedFunction(ltsname)
	if lifetimeStart.IsNil() {
		pts := []llvm.Type{
			llvm.Int64Type(),
			llvm.PointerType(llvm.Int8Type(), 0),
		}

		ft := llvm.FunctionType(llvm.VoidType(), pts, false)
		lifetimeStart = llvm.AddFunction(CompilationUnits.Peek().Module, ltsname, ft)
	}

	targetData := llvm.NewTargetData(CompilationUnits.Peek().Module.DataLayout())
	size := llvm.ConstInt(llvm.Int64Type(), targetData.TypeAllocSize(typ), false)
	args := []llvm.Value{
		size,
		Cast(alloca, llvm.PointerType(llvm.Int8Type(), 0)),
	}

	CompilationUnits.Peek().Builder.CreateCall(lifetimeStart.Type().ReturnType(), lifetimeStart, args, "")
	return alloca
}

func GetStructFieldPtr(strct llvm.Value, fieldname string) llvm.Value {
	if strct.Type().TypeKind() != llvm.PointerTypeKind {
		LogError("cannot access struct fields on a non-pointer type: `%s`", strct.Type().String())
	}

	strctname := strct.Type().ElementType().StructName()
	baseStruct := CompilationUnits.Peek().Structs[strctname]

	var field llvm.Value
	for i, f := range baseStruct.Fields {
		if fieldname == f.Name {
			field = CompilationUnits.Peek().Builder.CreateStructGEP(strct.Type().ElementType(), strct, i, "")
		}
	}

	return field
}
