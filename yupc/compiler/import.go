package compiler

import (
	"os"
	"os/exec"
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

	if filepath.Ext(name) == ".c" {
		bitcode := strings.TrimSuffix(name, filepath.Ext(".c")) + ".bc"
		dir := filepath.Dir(bitcode)
		fileName := filepath.Base(bitcode)
		bitcode = filepath.Join(dir, "build", fileName)

		cmdargs := []string{"-c", "-emit-llvm", name, "-o", bitcode}
		cmd := exec.Command("clang-14", cmdargs...)
		cmd.Stdin = os.Stdin
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		err := cmd.Run()

		if err != nil {
			LogError("compiling imported C failed (%s):\n %s", name, err.Error())
		}

		module, err := llvm.ParseBitcodeFile(bitcode)
		if err != nil {
			LogError("failed to parse Bitcode: %s", err.Error())
		}

		cu := NewCompilationUnit(name, name)
		cu.Module = &module
		CompilationUnits.Push(cu)
	} else {
		ProcessPathRecursively(name)
	}

	unit := CompilationUnits.Pop()
	for name, typ := range unit.Types {
		if _, ok := CompilationUnits.Peek().Types[name]; !ok && typ.IsPublic {
			CompilationUnits.Peek().Types[name] = &TypeInfo{
				Name: name,
				Type: typ.Type,
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
	// TODO: figure out why linking causes deformed names
	//llvm.LinkModules(*CompilationUnits.Peek().Module, *unit.Module)
}

func (v *AstVisitor) VisitImportDecl(ctx *parser.ImportDeclContext) any {
	for _, im := range ctx.AllValueString() {
		name, _ := strconv.Unquote(im.GetText())
		ImportModule(name)
	}

	return nil
}
