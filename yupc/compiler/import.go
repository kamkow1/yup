package compiler

import (
	"log"
	"strconv"
	"path/filepath"
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
		llvm.AddFunction(CompilationUnits.Peek().Module,
			mod.FirstFunction().Name(), mod.FirstFunction().Type())
	}

	if !mod.LastGlobal().IsNil() {
		llvm.AddGlobal(CompilationUnits.Peek().Module,
			mod.FirstGlobal().Type(), mod.FirstGlobal().Name())
	}

	mod.SetDataLayout(CompilationUnits.Peek().Module.DataLayout())
	llvm.LinkModules(CompilationUnits.Peek().Module, mod)
}

func (v *AstVisitor) VisitImportDeclaration(ctx *parser.ImportDeclarationContext) any {
	name, _ := strconv.Unquote(ctx.ValueString().GetText())
	ImportModule(name)
	return nil
}
