package compiler

import (
	"path"
	"strings"
	"unsafe"
)

// replce with Dequote()
func TrimLeftChar(s string) string {
	for i := range s {
		if i > 0 {
			return s[i:]
		}
	}
	return s[:0]
}

// replace with Dequote()
func TrimRightChar(s string) string {
	return s[:len(s)-1]
}

func FilenameWithoutExtension(fn string) string {
	return strings.TrimSuffix(fn, path.Ext(fn))
}

func BoolToInt(a bool) uint64 {
	return *(*uint64)(unsafe.Pointer(&a)) & 1
}
