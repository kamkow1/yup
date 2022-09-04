package cmd

import (
	"fmt"
	"io/ioutil"
	"os"
	"path"

	"github.com/kamkow1/yup/yupcgo/ast"
	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/spf13/cobra"
)

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
		ast.ProcessSourceFile(fileContent, abspath)
		compiler.DebugPrintModule()
	}
}

var compileCmd = &cobra.Command{
	Use:   "compile",
	Short: "compiles a single source file and it's dependencies",
	Run: func(cmd *cobra.Command, args []string) {
		fp := args[0]
		ProcessPathRecursively(fp)
	},
}

func init() {
	rootCmd.AddCommand(compileCmd)
}
