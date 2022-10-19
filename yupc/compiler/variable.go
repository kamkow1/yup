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
			value = Cast(value, typ)
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

	return CompilationUnits.Peek().Builder.CreateStore(value, vr.Value)
}

func (v *AstVisitor) VisitExprAssign(ctx *parser.ExprAssignContext) any {
	expr := v.Visit(ctx.Expression()).(llvm.Value)
	value := v.Visit(ctx.VarValue()).(llvm.Value)

	return CompilationUnits.Peek().Builder.CreateStore(value, expr)
}
