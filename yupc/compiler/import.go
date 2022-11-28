package compiler

import (
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

	ProcessPathRecursively(name)

	unit := CompilationUnits.Pop()
	for name, typ := range unit.Types {
		if _, ok := CompilationUnits.Peek().Types[name]; !ok && typ.IsPublic {

			// skip all interaces
			if !typ.IsInterf {
				CompilationUnits.Peek().Types[name] = &TypeInfo{
					Name: name,
					Type: typ.Type,
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
		// function doesn't exist so we can safely import it
		if _, ok := CompilationUnits.Peek().Functions[name]; !ok {

			function := unit.Module.NamedFunction(funcInfo.Name)
			returnType := function.Type().ReturnType().ReturnType()
			paramTypes := function.Type().ElementType().ParamTypes()
			vararg := function.Type().ElementType().IsFunctionVarArg()

			functionType := llvm.FunctionType(returnType, paramTypes, vararg)
			newFunction := llvm.AddFunction(*CompilationUnits.Peek().Module, funcInfo.Name, functionType)

			CompilationUnits.Peek().Functions[newFunction.Name()] = &Function{
				Name:       newFunction.Name(),
				Params:     funcInfo.Params,
				Value:      &newFunction,
				ExitBlock:  funcInfo.ExitBlock,
				MethodName: funcInfo.MethodName,
			}
		}
	}

	for name := range unit.Globals {
		if _, ok := CompilationUnits.Peek().Globals[name]; !ok {
			if CompilationUnits.Peek().Module.NamedGlobal(name).IsNil() {
				glb := unit.Module.NamedGlobal(name)
				glb = llvm.AddGlobal(*CompilationUnits.Peek().Module, glb.Type(), name)
				CompilationUnits.Peek().Globals[name] = &glb
			}
		}
	}

	unit.Module.SetDataLayout(CompilationUnits.Peek().Module.DataLayout())
	llvm.LinkModules(*CompilationUnits.Peek().Module, *unit.Module)
}

func (v *AstVisitor) VisitImportDecl(ctx *parser.ImportDeclContext) any {
	for _, im := range ctx.AllValueString() {
		name, _ := strconv.Unquote(im.GetText())
		ImportModule(name)
	}

	return nil
}
