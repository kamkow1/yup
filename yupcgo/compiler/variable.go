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

func CreateVariable(name string, typ llvm.Type, isGlobal bool) {
	if isGlobal {
		v := llvm.AddGlobal(compilationUnits.Peek().module, typ, name)
		gv := GlobalVariable{&Variable{name, false}, v}
		compilationUnits.Peek().globals[name] = gv

		compilationUnits.Peek().module.Dump()

	} else {
		fmt.Println("global")
		v := compilationUnits.Peek().builder.CreateAlloca(typ, name)
		lv := LocalVariable{&Variable{name, false}, v}
		(*compilationUnits.Peek().locals.Peek())[name] = lv
	}
}
