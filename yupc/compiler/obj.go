package compiler

import (
	"os/exec"
)

func DumpObjectFile(bcname string) {
	objname := FilenameWithoutExtension(bcname) + ".o"

	cmdargs := []string{"-c", "-o", objname, bcname}
	err := exec.Command("clang", cmdargs...).Run()
	if err != nil {
		LogError("failed to dump an object file: %s", bcname)
	}
}
