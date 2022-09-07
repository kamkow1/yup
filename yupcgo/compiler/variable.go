package compiler

import (
	"fmt"

	"tinygo.org/x/go-llvm"
)

type LocalVariable struct {
	Name    string
	IsConst bool
	Value   llvm.Value
}

func CreateVariable(name string, typ llvm.Type, isGlobal bool,
	isConstant bool, module *llvm.Module, builder *llvm.Builder) {
	var alreadyLocal bool
	if len(CompilationUnits.Peek().Locals) < 1 {
		alreadyLocal = false
	} else if _, ok := CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][name]; ok {
		alreadyLocal = ok
	}

	if alreadyLocal {
		panic(fmt.Sprintf("ERROR: variable %s: %s has already been declared",
			name, typ.String()))
	}

	if isGlobal {
		llvm.AddGlobal(*module, typ, name)
	} else {
		v := builder.CreateAlloca(typ, "")
		lv := LocalVariable{name, isConstant, v}
		CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][name] = lv
	}
}

func InitializeVariable(name string, value llvm.Value, isGlobal bool, builder *llvm.Builder) {
	if isGlobal {
		variable := CompilationUnits.Peek().Module.NamedGlobal(name)
		variable.SetInitializer(value)
	} else {
		variable := CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][name]
		builder.CreateStore(value, variable.Value)
	}
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

func GetVariable(name string, builder *llvm.Builder) llvm.Value {
	if !CompilationUnits.Peek().Module.NamedGlobal(name).IsNil() {
		v := CompilationUnits.Peek().Module.NamedGlobal(name)
		return builder.CreateLoad(v, "")
	}

	v := FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1).Value
	return builder.CreateLoad(v, "")
}
