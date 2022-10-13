package cmd

import (
	"github.com/kamkow1/yup/yupc/compiler"
	"github.com/spf13/cobra"
	"tinygo.org/x/go-llvm"
)

var compileCmd = &cobra.Command{
	Use:   "compile",
	Short: "compiles a single source file and it's dependencies",
	Run: func(cmd *cobra.Command, args []string) {

		llvm.InitializeAllTargetInfos()
		llvm.InitializeAllTargets()
		llvm.InitializeAllTargetMCs()
		llvm.InitializeAllAsmParsers()
		llvm.InitializeAllAsmPrinters()

		if verb, _ := cmd.Flags().GetBool("verbose"); verb {
			compiler.GlobalCompilerInfo.Verbose = verb
		}

		for _, fp := range args {
			compiler.ProcessPathRecursively(fp)
			mod, p := compiler.GetBCWriteData()
			compiler.WriteBCFile(mod, p)

			modname := compiler.CompilationUnits.Peek().ModuleName
			objpath := compiler.DumpObjectFile(modname)

			if execname, _ := cmd.Flags().GetString("output"); execname != "" {
				compiler.MakeExec(objpath, execname)
			}
		}
	},
}

func init() {
	compileCmd.PersistentFlags().String("output", "yup.out", "outputs an executable")
	compileCmd.PersistentFlags().Bool("verbose", false, "enables verbose output from external tools")
	rootCmd.AddCommand(compileCmd)
}
