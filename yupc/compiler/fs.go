package compiler

import (
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"strings"

	"github.com/antlr/antlr4/runtime/Go/antlr"
	"github.com/kamkow1/yup/yupc/lexer"
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func GetHomeDir() string {
	dir, err := os.UserHomeDir()
	if err != nil {
		panic(fmt.Sprintf("ERROR: %s", err))
	}

	return dir
}

func GetCwd() string {
	dir, err := os.Getwd()
	if err != nil {
		panic(fmt.Sprintf("ERROR: %s", err))
	}

	return dir
}

var DefaultImportPaths map[string]string = map[string]string{
	"$std":  path.Join(GetHomeDir(), "yup_stdlib"),
	"$root": GetCwd(),
}

func WriteBCFile(mod llvm.Module, p string) {
	if f, err := os.Create(p); err != nil {
		panic(fmt.Sprintf("ERROR: %s\n", err.Error()))
	} else if err2 := llvm.WriteBitcodeToFile(mod, f); err != nil {
		panic(fmt.Sprintf("ERROR: %s\n", err2.Error()))
	} else {
		defer f.Close()
	}
}

func GetBCFileName(fp string) string {
	ext := filepath.Ext(fp)
	bc := strings.Replace(fp, ext, ".bc", 1)
	cwd, err := os.Getwd()
	if err != nil {
		panic(fmt.Sprintf("ERROR: %s\n", err))
	}

	build := path.Join(cwd, "build")
	_, err2 := os.Stat(build)
	if os.IsNotExist(err2) {
		err3 := os.Mkdir(build, 0775)
		if err3 != nil {
			panic(fmt.Sprintf("ERROR: %s\n", err2))
		}
	}

	f := path.Base(bc)
	full := path.Join(build, f)
	return full
}

func ProcessSourceFile(file string, fp string, bcName string) {
	is := antlr.NewInputStream(file)
	lexer := lexer.NewYupLexer(is)
	tokens := antlr.NewCommonTokenStream(lexer, antlr.TokenDefaultChannel)
	parser := parser.NewYupParser(tokens)

	parser.BuildParseTrees = true
	tree := parser.File()
	v := NewAstVisitor()

	cu := NewCompilationUnit(fp, bcName)
	CompilationUnits.Push(cu)

	v.Visit(tree)
}

func ProcessPathRecursively(p string) {
	info, err := os.Stat(p)
	if err != nil {
		panic(fmt.Sprintf("ERROR: unable to process path: %s\n", p))
	}

	if info.IsDir() {
		items, _ := ioutil.ReadDir(p)
		for _, item := range items {
			ProcessPathRecursively(item.Name())
		}
	} else {
		abspath := path.Join(p)

		fileBytes, err := ioutil.ReadFile(abspath)
		if err != nil {
			panic(fmt.Sprintf("ERROR: an error has occurend when reading file: %s", abspath))
		}

		fileContent := string(fileBytes)
		ProcessSourceFile(fileContent, abspath, GetBCFileName(abspath))
	}
}

func ImportModule(name string) {
	elems := strings.Split(name, "/")
	if p, ok := DefaultImportPaths[elems[0]]; ok {
		name = strings.ReplaceAll(name, elems[0], p)
	}

	if filepath.Ext(name) == ".bc" {
		module, err := llvm.ParseBitcodeFile(name)
		if err != nil {
			panic(fmt.Sprintf("ERROR: import %s: %s", name, err))
		}

		cu := NewCompilationUnit(name, name)
		cu.Module = module
		CompilationUnits.Push(cu)
	} else if filepath.Ext(name) == ".ll" {
		ctx := &llvm.Context{}
		file, err := llvm.NewMemoryBufferFromFile(name)
		if err != nil {
			panic(fmt.Sprintf("ERROR: import %s: %s", name, err))
		}

		fmt.Println(string(file.Bytes()))

		module, err2 := ctx.ParseIR(file)
		if err2 != nil {
			panic(fmt.Sprintf("ERROR: import %s: %s", name, err2))
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

	llvm.LinkModules(CompilationUnits.Peek().Module, mod)
}
