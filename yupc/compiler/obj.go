package compiler

import (
	"os"
	"os/exec"
)

func DumpObjectFile(bcname string) string {
	objname := FilenameWithoutExtension(bcname) + ".o"

	cmdargs := []string{"-c", "-o", objname, bcname}
	if len(GlobalCompilerInfo.StaticLibs) > 0 {
		cmdargs = append(cmdargs, "-l")
		cmdargs = append(cmdargs, GlobalCompilerInfo.StaticLibs...)
	}
	cmd := exec.Command("clang-14", cmdargs...)

	if GlobalCompilerInfo.Verbose {
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		cmd.Stdin = os.Stdin
	}

	err := cmd.Run()

	if err != nil {
		LogError("failed to dump an object file: %s", bcname)
	}

	return objname
}

func MakeExec(objpath string, execname string) {
	cmdargs := []string{"-o", execname, objpath}
	cmd := exec.Command("clang-14", cmdargs...)

	if GlobalCompilerInfo.Verbose {
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		cmd.Stdin = os.Stdin
	}

	err := cmd.Run()

	if err != nil {
		LogError("failed to construct an executable")
	}
}
