package main

import (
	"syscall"

	"github.com/kamkow1/yup/yupcgo/cmd"
)

func main() {
	syscall.Umask(0)
	cmd.Execute()
}
