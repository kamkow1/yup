package compiler

import (
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
	if isGlobal {
		v := llvm.AddGlobal(compilationUnits.Peek().module, typ, name)
		var linkage llvm.Linkage
		if isExported {
			linkage = llvm.ExternalLinkage
		} else {
			linkage = llvm.PrivateLinkage
		}

		v.SetLinkage(linkage)
		gv := GlobalVariable{&Variable{name, isConstant}, v}
		compilationUnits.Peek().globals[name] = gv
	} else {
		v := compilationUnits.Peek().builder.CreateAlloca(typ, name)
		lv := LocalVariable{&Variable{name, isConstant}, v}
		(*compilationUnits.Peek().locals.Peek())[name] = lv
	}
}

func InitializeVariable(name string, value llvm.Value, isGlobal bool) {
	if isGlobal {
		variable := compilationUnits.Peek().globals[name]
		variable.value.SetInitializer(value)
		compilationUnits.Peek().module.Dump()
	}

}
