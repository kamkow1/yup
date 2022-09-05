package cmd

import (
	"github.com/kamkow1/yup/yupcgo/fs"
	"github.com/spf13/cobra"
)

var compileCmd = &cobra.Command{
	Use:   "compile",
	Short: "compiles a single source file and it's dependencies",
	Run: func(cmd *cobra.Command, args []string) {
		for _, fp := range args {
			fs.ProcessPathRecursively(fp)
		}
	},
}

func init() {
	rootCmd.AddCommand(compileCmd)
}
