package cmd

import (
	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/spf13/cobra"
)

var compileCmd = &cobra.Command{
	Use:   "compile",
	Short: "compiles a single source file and it's dependencies",
	Run: func(cmd *cobra.Command, args []string) {
		for _, fp := range args {
			compiler.ProcessPathRecursively(fp)
			mod, p := compiler.GetBCWriteData()
			compiler.WriteBCFile(mod, p)

		}
	},
}

func init() {
	rootCmd.AddCommand(compileCmd)
}
