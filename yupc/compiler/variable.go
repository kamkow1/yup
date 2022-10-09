package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

type LocalVariable struct {
	Name    string
	IsConst bool
	Value   llvm.Value
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

func (v *AstVisitor) VisitVariableValue(ctx *parser.VariableValueContext) any {
	return v.Visit(ctx.Expression())
}

func (v *AstVisitor) VisitDeclarationType(ctx *parser.DeclarationTypeContext) any {
	return ctx.KeywordConst() != nil
}

func (v *AstVisitor) VisitVariableDeclare(ctx *parser.VariableDeclareContext) any {
	name := ctx.Identifier().GetText()

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
	isConstant := v.Visit(ctx.DeclarationType()).(bool) // true == const, false == var

	var typ llvm.Type
	var value llvm.Value
	isInit := ctx.VariableValue() != nil
	if isInit {
		value = v.Visit(ctx.VariableValue()).(llvm.Value)
		typ = value.Type()
	}

	if ctx.TypeAnnotation() != nil {
		typ = v.Visit(ctx.TypeAnnotation()).(llvm.Type)
	} else {
		typ = value.Type()
	}

	if isGlobal {
		glb := llvm.AddGlobal(CompilationUnits.Peek().Module, typ, name)
		if isInit {
			glb.SetInitializer(value)
		}

		if ctx.AttributeList() != nil {
			attrs := v.Visit(ctx.AttributeList()).([]*Attribute)
			for _, a := range attrs {
				switch a.Name {
				case "link_type":
					{
						linkage := a.Params[0]
						glb.SetLinkage(GetLinkageFromString(linkage))
					}
				}
			}
		}

		CompilationUnits.Peek().Globals[name] = glb

	} else {
		if ctx.AttributeList() != nil {
			LogError("local variable %s cannot have an attribute list", name)
		}

		v := CompilationUnits.Peek().Builder.CreateAlloca(typ, "")
		lv := LocalVariable{name, isConstant, v}
		CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][name] = lv
		if isInit {
			value = Cast(value, typ)
			CompilationUnits.Peek().Builder.CreateStore(value, v)
		}
	}

	return nil
}

func (v *AstVisitor) VisitIdentifierExpression(ctx *parser.IdentifierExpressionContext) any {
	name := ctx.Identifier().GetText()

	if tp, ok := CompilationUnits.Peek().Types[name]; ok {
		return tp
	}

	var val llvm.Value
	if !CompilationUnits.Peek().Module.NamedGlobal(name).IsNil() {
		val = CompilationUnits.Peek().Module.NamedGlobal(name)
		return CompilationUnits.Peek().Builder.CreateLoad(val, "")
	}

	val = FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1).Value
	return CompilationUnits.Peek().Builder.CreateLoad(val, "")
}

func (v *AstVisitor) VisitAssignment(ctx *parser.AssignmentContext) any {
	name := ctx.Identifier().GetText()
	vr := FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1)
	if vr.IsConst {
		LogError("cannot reassign a constant: %s", vr.Name)
	}

	value := v.Visit(ctx.VariableValue()).(llvm.Value)

	return CompilationUnits.Peek().Builder.CreateStore(value, vr.Value)
}

func (v *AstVisitor) VisitExpressionAssignment(ctx *parser.ExpressionAssignmentContext) any {
	expr := v.Visit(ctx.Expression()).(llvm.Value)
	value := v.Visit(ctx.VariableValue()).(llvm.Value)

	return CompilationUnits.Peek().Builder.CreateStore(value, expr)
}
