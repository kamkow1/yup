package compiler

import (
	"fmt"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

type LocalVariable struct {
	Name    string
	IsConst bool
	Value   llvm.Value
}

func FindLocalVariable(name string, i int) LocalVariable {
	if v, ok := CompilationUnits.Peek().Locals[i][name]; ok {
		return v
	} else if i >= 1 {
		return FindLocalVariable(name, i-1)
	} else {
		panic(fmt.Sprintf("ERROR: tried to reference an unknown variable: %s", name))
	}
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
		panic(fmt.Sprintf("ERROR: variable %s has already been declared", name))
	}

	isGlobal := ctx.KeywordGlobal() != nil
	isConstant := v.Visit(ctx.DeclarationType()).(bool) // true == const, false == var

	var typ llvm.Type
	var value llvm.Value
	isInit := false
	if ctx.TypeAnnotation() != nil {
		typ = v.Visit(ctx.TypeAnnotation()).(llvm.Type)
	} else {
		value = v.Visit(ctx.VariableValue()).(llvm.Value)
		typ = value.Type()
		isInit = true
	}

	if isGlobal {
		glb := llvm.AddGlobal(CompilationUnits.Peek().Module, typ, name)
		if isInit {
			glb.SetInitializer(value)
		}
	} else {
		v := CompilationUnits.Peek().Builder.CreateAlloca(typ, "")
		lv := LocalVariable{name, isConstant, v}
		CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][name] = lv
		if isInit {
			CompilationUnits.Peek().Builder.CreateStore(value, v)
		}
	}

	return nil
}

func (v *AstVisitor) VisitIdentifierExpression(ctx *parser.IdentifierExpressionContext) any {
	name := ctx.Identifier().GetText()
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
	value := v.Visit(ctx.VariableValue()).(llvm.Value)

	vr := FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1).Value
	return CompilationUnits.Peek().Builder.CreateStore(value, vr)
}
