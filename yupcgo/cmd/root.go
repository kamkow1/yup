package cmd

import (
	"os"

	"github.com/spf13/cobra"
)

var rootCmd = &cobra.Command{
	Use:   "yupc",
	Short: "compiler of the yup programming language",
	Long: `yupc was originally written in C++ with LLVM. 
this version of the compiler is written in Go with LLVM bindings (because C++ sucks).`,
	Run: func(cmd *cobra.Command, args []string) {},
}

func Execute() {
	err := rootCmd.Execute()
	if err != nil {
		os.Exit(1)
	}
}

func init() {
	rootCmd.AddCommand(compileCmd)
}
