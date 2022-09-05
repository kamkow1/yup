package fs

import (
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"strings"

	"github.com/kamkow1/yup/yupcgo/ast"
	"github.com/kamkow1/yup/yupcgo/compiler"
	"tinygo.org/x/go-llvm"
)

func WriteBCFile(mod llvm.Module, p string) {
	if f, err := os.Create(p); err != nil {
		panic(fmt.Sprintf("ERROR: %s", err.Error()))
	} else if err2 := llvm.WriteBitcodeToFile(mod, f); err != nil {
		panic(fmt.Sprintf("ERROR: %s", err2.Error()))
	} else {
		defer f.Close()
	}
}

func GetBCFileName(fp string) string {
	ext := filepath.Ext(fp)
	bc := strings.Replace(fp, ext, ".bc", 1)
	cwd, err := os.Getwd()
	if err != nil {
		panic(fmt.Sprintf("%s\n", err))
	}

	build := path.Join(cwd, "build")
	_, err2 := os.Stat(build)
	if os.IsNotExist(err2) {
		err3 := os.Mkdir(build, 0775)
		if err3 != nil {
			panic(fmt.Sprintf("%s\n", err2))
		}
	}

	f := path.Base(bc)
	full := path.Join(build, f)
	return full
}

func ProcessPathRecursively(p string) {
	info, err := os.Stat(p)
	if err != nil {
		panic(fmt.Sprintf("ERROR: unable to process path: %s", p))
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
		ast.ProcessSourceFile(fileContent, abspath, GetBCFileName(abspath))
		mod, p := compiler.GetBCWriteData()
		WriteBCFile(mod, p)
	}
}
