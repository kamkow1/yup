package compiler

import (
	"encoding/binary"

	"tinygo.org/x/go-llvm"
)

const UnsignedIntMin = 0

func GetIntegerConstant(value int64) llvm.Value {
	size := binary.Size(value)
	typ := llvm.IntType(int(size))
	return llvm.ConstInt(typ, uint64(value), false)
}

func GetFloatConstant(value float64) llvm.Value {
	typ := llvm.FloatType()
	return llvm.ConstFloat(typ, value)
}

func GetCharConstant(value byte) llvm.Value {
	typ := llvm.Int8Type()
	return llvm.ConstInt(typ, uint64(value), false)
}

func GetStringConstant(value string) llvm.Value {
	return llvm.ConstString(value, true)
}
