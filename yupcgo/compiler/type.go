package compiler

import (
	"fmt"
	"strconv"

	"github.com/kamkow1/yup/yupcgo/parser"
	"tinygo.org/x/go-llvm"
)

var BuiltinLLVMTypes map[string]llvm.Type

func init() {
	BuiltinLLVMTypes = map[string]llvm.Type{
		"i1":   llvm.Int1Type(),
		"i8":   llvm.Int8Type(),
		"i16":  llvm.Int16Type(),
		"i32":  llvm.Int32Type(),
		"i64":  llvm.Int64Type(),
		"f32":  llvm.FloatType(),
		"f128": llvm.FP128Type(),
		"dbl":  llvm.DoubleType(),
		"void": llvm.VoidType(),
	}
}

func GetPointerType(typ llvm.Type) llvm.Type {
	return llvm.PointerType(typ, 0)
}

func GetArrayType(typ llvm.Type, count int) llvm.Type {
	return llvm.ArrayType(typ, count)
}

func GetTypeFromName(name string) llvm.Type {
	if llvmType, ok := BuiltinLLVMTypes[name]; ok {
		return llvmType
	}

	panic(fmt.Sprintf("ERROR: unknown type: %s", name))
}

func AssertType(typ1 llvm.Type, typ2 llvm.Type) bool {
	return typ1.TypeKind() != typ2.TypeKind()
}

func (v *AstVisitor) VisitTypeName(ctx *parser.TypeNameContext) any {
	typ := GetTypeFromName(ctx.Identifier().GetText())
	for _, ext := range ctx.AllTypeExtension() {
		extension := ext.(*parser.TypeExtensionContext)
		if extension.SymbolAsterisk() != nil {
			typ = GetPointerType(typ)
		}

		if extension.ArrayTypeExtension() != nil {
			extCtx := extension.ArrayTypeExtension().(*parser.ArrayTypeExtensionContext)
			size, err := strconv.Atoi(extCtx.ValueInteger().GetText())
			if err != nil {
				panic(fmt.Sprintf("ERROR: failed to parse array size: %d, %s", size, err.Error()))
			}

			typ = GetArrayType(typ, size)
		}
	}

	return typ
}

func (v *AstVisitor) VisitTypeAnnotation(ctx *parser.TypeAnnotationContext) any {
	return v.Visit(ctx.TypeName())
}
