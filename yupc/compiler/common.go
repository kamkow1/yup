package compiler

import (
	"tinygo.org/x/go-llvm"
)

func GetLinkageFromString(linkage string) llvm.Linkage {
	switch linkage {
	case "once":
		return llvm.LinkOnceAnyLinkage
	case "private":
		return llvm.PrivateLinkage
	case "common":
		return llvm.CommonLinkage
	case "internal":
		return llvm.InternalLinkage
	case "weak":
		return llvm.WeakAnyLinkage
	case "extern-elf":
		return llvm.ExternalWeakLinkage
	}

	LogError("unknown linkage type in attribute: %s", linkage)
	return 0
}

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
	isptr := strct.Type().TypeKind() == llvm.PointerTypeKind

	var strctname string
	if isptr {
		strctname = strct.Type().ElementType().StructName()
	} else {
		strctname = strct.Type().StructName()
	}

	baseStruct, _ := CompilationUnits.Peek().Structs[strctname]

	var field llvm.Value
	for i, f := range baseStruct.Fields {
		if fieldname == f.Name {
			field = CompilationUnits.Peek().Builder.CreateStructGEP(
				field.Type().ElementType(), strct, i, "")
		}
	}

	return field
}
