package compiler

import (
	"fmt"

	"tinygo.org/x/go-llvm"
)

type Variable struct {
	Name    string
	IsConst bool
}

type LocalVariable struct {
	*Variable
	Value llvm.Value
}

type GlobalVariable struct {
	*Variable
	Value llvm.Value
}

func CreateVariable(name string, typ llvm.Type, isGlobal bool, isConstant bool,
	isExported bool, module *llvm.Module, builder *llvm.Builder) {
	_, alreadyGlobal := CompilationUnits.Peek().Globals[name]
	_, alreadyLocal := CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][name]
	if alreadyGlobal || alreadyLocal {
		panic(fmt.Sprintf("ERROR: variable %s: %s has already been declared",
			name, typ.String()))
	}

	if isGlobal {
		v := llvm.AddGlobal(*module, typ, name)
		var linkage llvm.Linkage
		if isExported {
			linkage = llvm.ExternalLinkage
		} else {
			linkage = llvm.PrivateLinkage
		}

		v.SetLinkage(linkage)
		gv := &GlobalVariable{&Variable{name, isConstant}, v}
		CompilationUnits.Peek().Globals[name] = gv
	} else {
		v := builder.CreateAlloca(typ, "")
		lv := LocalVariable{&Variable{name, isConstant}, v}
		CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][name] = lv
	}
}

func InitializeVariable(name string, value llvm.Value, isGlobal bool, builder *llvm.Builder) {
	if isGlobal {
		variable := CompilationUnits.Peek().Globals[name]
		variable.Value.SetInitializer(value)
	} else {
		variable := CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][name]
		builder.CreateStore(value, variable.Value)
	}
}

func FindLocalVariable(name string, i int) LocalVariable {
	if v, ok := CompilationUnits.Peek().Locals[i][name]; ok {
		return v
	} else if i > 0 {
		return FindLocalVariable(name, i-1)
	} else {
		panic(fmt.Sprintf("ERROR: tried to reference an unknown variable: %s", name))
	}
}

func GetVariable(name string, builder *llvm.Builder) llvm.Value {
	if v, ok := CompilationUnits.Peek().Globals[name]; ok {
		return builder.CreateLoad(v.Value, "")
	}

	v := FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1).Value
	return builder.CreateLoad(v, "")
}
