package main

import (
	"syscall"

	"github.com/kamkow1/yup/yupc/cmd"
)

func main() {
	syscall.Umask(0)
	cmd.Execute()
}
