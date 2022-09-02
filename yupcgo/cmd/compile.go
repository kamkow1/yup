package cmd

import (
	"fmt"
	"io/ioutil"
	"os"
	"path"

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
			fmt.Fprintf(os.Stderr, "ERROR: an error has occurend when reading file: %s", abspath)
			os.Exit(1)
		}

		fileContent := string(fileBytes)
		fmt.Printf("%s\n", fileContent)
	},
}

func init() {
	rootCmd.AddCommand(compileCmd)
}
