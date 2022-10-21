package compiler

import (
	"path/filepath"
	"strconv"
	"strings"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func ImportModule(name string) {
	elems := strings.Split(name, "/")
	if p, ok := DefaultImportPaths[elems[0]]; ok {
		name = strings.ReplaceAll(name, elems[0], p)
	}

	if filepath.Ext(name) == ".bc" {
		module, err := llvm.ParseBitcodeFile(name)
		if err != nil {
			LogError("failed to import %s: %s", name, err)
		}

		cu := NewCompilationUnit(name, name)
		cu.Module = module
		CompilationUnits.Push(cu)
	} else if filepath.Ext(name) == ".ll" {
		ctx := &llvm.Context{}
		file, err := llvm.NewMemoryBufferFromFile(name)
		if err != nil {
			LogError("failedt to import %s: %s", name, err)
		}

		module, err2 := ctx.ParseIR(file)
		if err2 != nil {
			LogError("failed to import %s: %s", name, err2)
		}

		cu := NewCompilationUnit(name, name)
		cu.Module = module
		CompilationUnits.Push(cu)
	} else {
		ProcessPathRecursively(name)
	}

	unit := CompilationUnits.Pop()
	mod := unit.Module

	/*for name, typ := range unit.Types {
		CompilationUnits.Peek().Types[name] = typ
	}*/

	for name, strct := range unit.Structs {
		CompilationUnits.Peek().Structs[name] = strct
		CompilationUnits.Peek().Types[name] = strct.Type
	}

	for name, typ := range unit.Types {
		if _, ok := CompilationUnits.Peek().Types[name]; !ok {
			CompilationUnits.Peek().Types[name] = typ
		}
	}

	for name, _ := range unit.Functions {
		fnc := mod.NamedFunction(name)
		fnc = llvm.AddFunction(CompilationUnits.Peek().Module, name, fnc.Type().ElementType())
		CompilationUnits.Peek().Functions[name] = Function{
			Name:      name,
			Params:    make([]FuncParam, 0),
			Value:     &fnc,
			ExitBlock: nil,
		}
	}

	for name, _ := range unit.Globals {
		glb := mod.NamedGlobal(name)
		glb = llvm.AddGlobal(CompilationUnits.Peek().Module, glb.Type(), name)
		CompilationUnits.Peek().Globals[name] = glb
	}

	mod.SetDataLayout(CompilationUnits.Peek().Module.DataLayout())
	llvm.LinkModules(CompilationUnits.Peek().Module, mod)
}

func (v *AstVisitor) VisitImportDecl(ctx *parser.ImportDeclContext) any {
	for _, im := range ctx.AllValueString() {
		name, _ := strconv.Unquote(im.GetText())
		ImportModule(name)
	}

	return nil
}
