package compiler

import (
	"io/ioutil"
	"log"
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
		log.Fatalf("ERROR: %s", err)
	}

	return dir
}

func GetCwd() string {
	dir, err := os.Getwd()
	if err != nil {
		log.Fatalf("ERROR: %s", err)
	}

	return dir
}

var DefaultImportPaths map[string]string = map[string]string{
	"$std":  path.Join(GetHomeDir(), "yup_stdlib"),
	"$root": GetCwd(),
}

func WriteBCFile(mod llvm.Module, p string) {
	if f, err := os.Create(p); err != nil {
		log.Fatalf("ERROR: %s\n", err.Error())
	} else if err2 := llvm.WriteBitcodeToFile(mod, f); err != nil {
		log.Fatalf("ERROR: %s\n", err2.Error())
	} else {
		defer f.Close()
	}
}

func GetBCFileName(fp string) string {
	ext := filepath.Ext(fp)
	bc := strings.Replace(fp, ext, ".bc", 1)
	cwd, err := os.Getwd()
	if err != nil {
		log.Fatalf("ERROR: %s\n", err)
	}

	build := path.Join(cwd, "build")
	_, err2 := os.Stat(build)
	if os.IsNotExist(err2) {
		err3 := os.Mkdir(build, 0775)
		if err3 != nil {
			log.Fatalf("ERROR: %s\n", err2)
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
	// llvm.VerifyModule(CompilationUnits.Peek().Module, llvm.PrintMessageAction)
}

func ProcessPathRecursively(p string) {
	info, err := os.Stat(p)
	if err != nil {
		log.Fatalf("ERROR: unable to process path: %s\n", p)
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
			log.Fatalf("ERROR: an error has occurend when reading file: %s", abspath)
		}

		fileContent := string(fileBytes)
		ProcessSourceFile(fileContent, abspath, GetBCFileName(abspath))
	}
}
