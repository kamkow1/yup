package compiler

import (
	"fmt"

	"tinygo.org/x/go-llvm"
)

type Variable struct {
	name    string
	isConst bool
}

type LocalVariable struct {
	*Variable
	value llvm.Value
}

type GlobalVariable struct {
	*Variable
	value llvm.Value
}

func CreateVariable(name string, typ llvm.Type, isGlobal bool, isConstant bool, isExported bool) {
	_, alreadyGlobal := compilationUnits.Peek().globals[name]
	_, alreadyLocal := compilationUnits.Peek().locals[len(compilationUnits.Peek().locals)-1][name]
	if alreadyGlobal || alreadyLocal {
		panic(fmt.Sprintf("ERROR: variable %s: %s has already been declared",
			name, typ.String()))
	}

	if isGlobal {
		v := llvm.AddGlobal(compilationUnits.Peek().module, typ, name)
		var linkage llvm.Linkage
		if isExported {
			linkage = llvm.ExternalLinkage
		} else {
			linkage = llvm.PrivateLinkage
		}

		v.SetLinkage(linkage)
		gv := &GlobalVariable{&Variable{name, isConstant}, v}
		compilationUnits.Peek().globals[name] = gv
	} else {
		v := compilationUnits.Peek().builder.CreateAlloca(typ, "")
		lv := LocalVariable{&Variable{name, isConstant}, v}
		compilationUnits.Peek().locals[len(compilationUnits.Peek().locals)-1][name] = lv
	}
}

func InitializeVariable(name string, value llvm.Value, isGlobal bool) {
	if isGlobal {
		variable := compilationUnits.Peek().globals[name]
		variable.value.SetInitializer(value)
	} else {
		variable := compilationUnits.Peek().locals[len(compilationUnits.Peek().locals)-1][name]
		compilationUnits.Peek().builder.CreateStore(value, variable.value)
	}
}

func FindLocalVariable(name string, i int) LocalVariable {
	if v, ok := compilationUnits.Peek().locals[i][name]; ok {
		return v
	} else if i > 0 {
		return FindLocalVariable(name, i-1)
	} else {
		panic(fmt.Sprintf("ERROR: tried to reference an unknown variable: %s", name))
	}
}

func GetVariable(name string) llvm.Value {
	if v, ok := compilationUnits.Peek().globals[name]; ok {
		return compilationUnits.Peek().builder.CreateLoad(v.value, "")
	}

	v := FindLocalVariable(name, len(compilationUnits.Peek().locals)-1).value
	return compilationUnits.Peek().builder.CreateLoad(v, "")
}
