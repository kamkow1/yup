package compiler

import (
	"tinygo.org/x/go-llvm"
)

func GetLinkageFromString(linkage string) llvm.Linkage {
	switch linkage {
	case "once":
		return llvm.LinkOnceAnyLinkage
	case "private":
		return llvm.PrivateLinkage
	case "common":
		return llvm.CommonLinkage
	case "internal":
		return llvm.InternalLinkage
	case "weak":
		return llvm.WeakAnyLinkage
	case "extern-elf":
		return llvm.ExternalWeakLinkage
	}

	LogError("unknown linkage type in attribute: %s", linkage)
	return 0
}
