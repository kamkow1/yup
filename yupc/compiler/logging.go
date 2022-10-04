package compiler

import (
	"fmt"
	"os"
)

func LogError(f string, vars ...any) {
	base := fmt.Sprintf("%s:%d", GlobalCompilerInfo.File, GlobalCompilerInfo.Line)
	msg := fmt.Sprintf(f, vars...)

	fmt.Println(fmt.Sprintf("%s %s", base, msg))
	os.Exit(1)
}
