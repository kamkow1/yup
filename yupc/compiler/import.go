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

	for name, typ := range unit.Types {
		if typ.IsPublic {
			newType := CompilationUnits.Peek().Module.Context().StructCreateNamed(name)
			newType.StructSetBody(typ.Type.StructElementTypes(), false)
			_, ok := CompilationUnits.Peek().Types[name]
			if !ok {
				CompilationUnits.Peek().Types[name] = &TypeInfo{
					Name: name,
					Type: newType,
				}
			}
		}
	}

	for name, strct := range unit.Structs {
		if _, ok := CompilationUnits.Peek().Structs[name]; !ok && strct.IsPublic {
			CompilationUnits.Peek().Structs[name] = strct
		}
	}

	for name, funcInfo := range unit.Functions {
		function := mod.NamedFunction(funcInfo.Value.Name())

		if !function.IsNil() {
			functionType := function.Type()
			if functionType.TypeKind() == llvm.PointerTypeKind {
				functionType = functionType.ElementType()

				if functionType.TypeKind() == llvm.FunctionTypeKind {
					addedFunction := llvm.AddFunction(CompilationUnits.Peek().Module, funcInfo.Value.Name(), functionType)
					CompilationUnits.Peek().Functions[funcInfo.Value.Name()] = &Function{
						Name:      name,
						Params:    funcInfo.Params,
						Value:     &addedFunction,
						ExitBlock: nil,
					}
				}
			}
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
