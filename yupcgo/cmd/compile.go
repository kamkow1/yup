package cmd

import (
	"fmt"
	"io/ioutil"
	"path"

	"github.com/kamkow1/yup/yupcgo/ast"
	"github.com/spf13/cobra"
)

var compileCmd = &cobra.Command{
	Use:   "compile",
	Short: "compiles a single source file and it's dependencies",
	Run: func(cmd *cobra.Command, args []string) {
		fp := args[0]
		abspath := path.Join(fp)

		fileBytes, err := ioutil.ReadFile(abspath)
		if err != nil {
			panic(fmt.Sprintf("ERROR: an error has occurend when reading file: %s", abspath))
		}

		fileContent := string(fileBytes)

		ast.ProcessSourceFile(fileContent, fp)
	},
}

func init() {
	rootCmd.AddCommand(compileCmd)
}
