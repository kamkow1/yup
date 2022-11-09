package compiler

import (
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"strings"

	"github.com/antlr/antlr4/runtime/Go/antlr/v4"
	"github.com/kamkow1/yup/yupc/lexer"
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func GetHomeDir() string {
	dir, err := os.UserHomeDir()
	if err != nil {
		LogError(err.Error())
	}

	return dir
}

func GetCwd() string {
	dir, err := os.Getwd()
	if err != nil {
		LogError(err.Error())
	}

	return dir
}

var DefaultImportPaths map[string]string = map[string]string{
	"#std":     path.Join(GetHomeDir(), "yup_stdlib"),
	"#project": GetCwd(),
}

func WriteBCFile(mod *llvm.Module, p string) {
	if f, err := os.Create(p); err != nil {
		LogError(err.Error())
	} else {
		defer f.Close()

		if err := llvm.WriteBitcodeToFile(*mod, f); err != nil {
			LogError(err.Error())
		}
	}
}

func GetBCFileName(fp string) string {
	ext := filepath.Ext(fp)
	bc := strings.Replace(fp, ext, ".bc", 1)
	cwd, err := os.Getwd()
	if err != nil {
		LogError(err.Error())
	}

	build := path.Join(cwd, "build")
	_, err2 := os.Stat(build)
	if os.IsNotExist(err2) {
		err3 := os.Mkdir(build, 0775)
		if err3 != nil {
			LogError(err3.Error())
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
	// runtime.KeepAlive(cu)

	v.Visit(tree)

	// set all inline assembly for the module
	CompilationUnits.Peek().Module.SetInlineAsm(InlineAsm)

	if GlobalCompilerInfo.PrintModule {
		CompilationUnits.Peek().Module.Dump()
	}

	if err := llvm.VerifyModule(*CompilationUnits.Peek().Module, llvm.PrintMessageAction); err != nil {
		LogError("verifying module `%s` failed. read error messages above", CompilationUnits.Peek().SourceFile)
	}
}

func ProcessPathRecursively(p string) {
	info, err := os.Stat(p)
	if err != nil {
		LogError("unable to process path: %s", p)
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
			LogError("an error has occurend when reading file: %s", abspath)
		}

		fileContent := string(fileBytes)
		ProcessSourceFile(fileContent, abspath, GetBCFileName(abspath))
	}
}
