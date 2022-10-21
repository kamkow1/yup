package cmd

import (
	"github.com/kamkow1/yup/yupc/compiler"
	"github.com/spf13/cobra"
)

var compileCmd = &cobra.Command{
	Use:   "compile",
	Short: "compiles a single source file and it's dependencies",
	Run: func(cmd *cobra.Command, args []string) {
		compiler.RunCompileJob(args)
	},
}

func init() {
	compileCmd.Flags().StringVarP(&compiler.GlobalCompilerInfo.OutName,
		"output", "o", "yup_program", "outputs an executable")
	compileCmd.Flags().BoolVarP(&compiler.GlobalCompilerInfo.Verbose,
		"verbose", "v", false, "enables verbose output from external tools")
	compileCmd.Flags().BoolVarP(&compiler.GlobalCompilerInfo.PrintModule,
		"print-module", "p", false, "prints module for debug purposes")
	compileCmd.Flags().StringArrayVarP(&compiler.GlobalCompilerInfo.StaticLibs,
		"static-libs", "l", make([]string, 0), "adds a static library to link against")
	rootCmd.AddCommand(compileCmd)
}
