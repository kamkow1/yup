package compiler

import (
	"log"
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
			log.Fatalf("ERROR: import %s: %s", name, err)
		}

		cu := NewCompilationUnit(name, name)
		cu.Module = module
		CompilationUnits.Push(cu)
	} else if filepath.Ext(name) == ".ll" {
		ctx := &llvm.Context{}
		file, err := llvm.NewMemoryBufferFromFile(name)
		if err != nil {
			log.Fatalf("ERROR: import %s: %s", name, err)
		}

		module, err2 := ctx.ParseIR(file)
		if err2 != nil {
			log.Fatalf("ERROR: import %s: %s", name, err2)
		}

		cu := NewCompilationUnit(name, name)
		cu.Module = module
		CompilationUnits.Push(cu)
	} else {
		ProcessPathRecursively(name)
	}

	mod := CompilationUnits.Pop().Module
	if !mod.LastFunction().IsNil() {
		f := mod.FirstFunction()
		llvm.AddFunction(CompilationUnits.Peek().Module,
			f.Name(), f.Type().ElementType())
	}

	if !mod.LastGlobal().IsNil() {
		g := mod.FirstGlobal()
		llvm.AddGlobal(CompilationUnits.Peek().Module,
			g.Type(), g.Name())
	}

	mod.SetDataLayout(CompilationUnits.Peek().Module.DataLayout())
	llvm.LinkModules(CompilationUnits.Peek().Module, mod)
}

func (v *AstVisitor) VisitImportDeclaration(ctx *parser.ImportDeclarationContext) any {
	for _, im := range ctx.AllValueString() {
		name, _ := strconv.Unquote(im.GetText())
		ImportModule(name)
	}

	return nil
}
