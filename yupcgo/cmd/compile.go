package cmd

import (
	"github.com/kamkow1/yup/yupcgo/ast"
	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/spf13/cobra"
)

var compileCmd = &cobra.Command{
	Use:   "compile",
	Short: "compiles a single source file and it's dependencies",
	Run: func(cmd *cobra.Command, args []string) {
		for _, fp := range args {
			ast.ProcessPathRecursively(fp)
			mod, p := compiler.GetBCWriteData()
			ast.WriteBCFile(mod, p)

		}
	},
}

func init() {
	rootCmd.AddCommand(compileCmd)
}
