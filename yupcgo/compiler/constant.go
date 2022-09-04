package compiler

import (
	"encoding/binary"

	"tinygo.org/x/go-llvm"
)

const UnsignedIntMin = 0

func GetIntegerConstant(value int64) llvm.Value {
	size := binary.Size(value)
	typ := compilationUnits.Peek().context.IntType(int(size))
	return llvm.ConstInt(typ, uint64(value), false)
}

func GetFloatConstant(value float64) llvm.Value {
	typ := compilationUnits.Peek().context.FloatType()
	return llvm.ConstFloat(typ, value)
}

func GetCharConstant(value byte) llvm.Value {
	typ := compilationUnits.Peek().context.Int8Type()
	return llvm.ConstInt(typ, uint64(value), false)
}

func GetStringConstant(value string) llvm.Value {
	return compilationUnits.Peek().context.ConstString(value, true)
}
