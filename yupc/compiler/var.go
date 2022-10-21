package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

type LocalVariable struct {
	Name    string
	IsConst bool
	Value   llvm.Value
	IsUsed  bool
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
		Cast(alloca, &TypeInfo{
    		     Type: llvm.PointerType(llvm.Int8Type(), 0),
    		}),
	}

	CompilationUnits.Peek().Builder.CreateCall(lifetimeStart.Type().ReturnType(), lifetimeStart, args, "")
	return alloca
}

func FindLocalVariable(name string, i int) LocalVariable {

	var local LocalVariable
	if v, ok := CompilationUnits.Peek().Locals[i][name]; ok {
		local = v
	} else if i >= 1 {
		local = FindLocalVariable(name, i-1)
	} else {
		LogError("tried to reference an unknown variable: %s", name)
	}

	return local
}

func (v *AstVisitor) VisitVarValue(ctx *parser.VarValueContext) any {
	return v.Visit(ctx.Expression())
}

func (v *AstVisitor) VisitDeclType(ctx *parser.DeclTypeContext) any {
	return ctx.KeywordConst() != nil
}

func (v *AstVisitor) VisitVarDecl(ctx *parser.VarDeclContext) any {
	var names []string
	for _, id := range ctx.AllIdentifier() {
		names = append(names, id.GetText())
	}

	for _, name := range names {
		var alreadyLocal bool
		if len(CompilationUnits.Peek().Locals) < 1 {
			alreadyLocal = false
		} else if _, ok := CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][name]; ok {
			alreadyLocal = ok
		}

		if alreadyLocal {
			LogError("variable %s has already been declared", name)
		}

		isGlobal := CompilationUnits.Peek().Builder.GetInsertBlock().IsNil()
		isConstant := v.Visit(ctx.DeclType()).(bool) // true == const, false == var

		var typ llvm.Type
		var value llvm.Value
		isInit := ctx.VarValue() != nil
		if isInit {
			value = v.Visit(ctx.VarValue()).(llvm.Value)
			typ = value.Type()
		}

		hasAnnot := ctx.TypeAnnot() != nil
		if hasAnnot {
			typ = v.Visit(ctx.TypeAnnot()).(llvm.Type)
		}

		if isInit && hasAnnot && value.Type() != typ {
			value = Cast(value, &TypeInfo{
				Type: typ,
    			})
		}

		if isGlobal {
			glb := llvm.AddGlobal(CompilationUnits.Peek().Module, typ, name)
			if isInit {
				glb.SetInitializer(value)
			}

			if ctx.KeywordPublic() == nil {
				glb.SetLinkage(llvm.PrivateLinkage)
			} else {
				glb.SetLinkage(llvm.LinkOnceAnyLinkage)
			}

			CompilationUnits.Peek().Globals[name] = glb

		} else {
			if ctx.AttrList() != nil {
				LogError("local variable %s cannot have an attribute list", name)
			}

			alloca := CreateAllocation(typ)
			loclen := len(CompilationUnits.Peek().Locals) - 1
			CompilationUnits.Peek().Locals[loclen][name] = LocalVariable{
				Name:    name,
				IsConst: isConstant,
				Value:   alloca,
			}

			if isInit {
				CompilationUnits.Peek().Builder.CreateStore(value, alloca)
			}
		}
	}

	return nil
}

func (v *AstVisitor) VisitIdentifierExpr(ctx *parser.IdentifierExprContext) any {
	name := ctx.Identifier().GetText()

	if typ, ok := CompilationUnits.Peek().Types[name]; ok {
		return typ
	}

	var val llvm.Value
	if !CompilationUnits.Peek().Module.NamedFunction(name).IsNil() {
		val = CompilationUnits.Peek().Module.NamedFunction(name)
		return CompilationUnits.Peek().Builder.CreateLoad(val.Type().ElementType(), val, "")
	}

	if !CompilationUnits.Peek().Module.NamedGlobal(name).IsNil() {
		val = CompilationUnits.Peek().Module.NamedGlobal(name)
		return CompilationUnits.Peek().Builder.CreateLoad(val.Type().ElementType(), val, "")
	}

	val = FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1).Value
	return CompilationUnits.Peek().Builder.CreateLoad(val.AllocatedType(), val, "")
}

func (v *AstVisitor) VisitAssign(ctx *parser.AssignContext) any {
	name := ctx.Identifier().GetText()
	vr := FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1)
	if vr.IsConst {
		LogError("cannot reassign a constant: %s", vr.Name)
	}

	value := v.Visit(ctx.VarValue()).(llvm.Value)
	vr.IsUsed = true

	if value.Type() == vr.Value.Type() {
		LogError("cannot assign type `%s` to `%s`", value.Type().String(), vr.Value.AllocatedType().String())
	}

	return CompilationUnits.Peek().Builder.CreateStore(value, vr.Value)
}

func (v *AstVisitor) VisitExprAssign(ctx *parser.ExprAssignContext) any {
	expr := v.Visit(ctx.Expression()).(llvm.Value)
	value := v.Visit(ctx.VarValue()).(llvm.Value)

	if expr.Type().ElementType() != value.Type() {
    		LogError("cannot assign type `%s` to `%s`", expr.Type().String(), value.Type().String())
	}

	return CompilationUnits.Peek().Builder.CreateStore(value, expr)
}

func (v *AstVisitor) VisitAddressOf(ctx *parser.AddressOfContext) any {
	name := ctx.Identifier().GetText()
	if !CompilationUnits.Peek().Module.NamedFunction(name).IsNil() {
		return CompilationUnits.Peek().Module.NamedFunction(name)
	}

	if !CompilationUnits.Peek().Module.NamedGlobal(name).IsNil() {
		return CompilationUnits.Peek().Module.NamedGlobal(name)
	}

	return FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1).Value
}

func (v *AstVisitor) VisitPtrDerefExpr(ctx *parser.PtrDerefExprContext) any {
	ptr := v.Visit(ctx.Expression()).(llvm.Value)
	if ptr.Type().TypeKind() != llvm.PointerTypeKind {
		LogError("cannot dereference a non-pointer type")
	}

	return CompilationUnits.Peek().Builder.CreateLoad(ptr.Type().ElementType(), ptr, "")
}
