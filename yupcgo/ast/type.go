package ast

import (
	"fmt"
	"strconv"

	"github.com/kamkow1/yup/yupcgo/compiler"
	"github.com/kamkow1/yup/yupcgo/parser"
)

func (v *AstVisitor) VisitTypeName(ctx *parser.TypeNameContext) any {
	typ := compiler.GetTypeFromName(ctx.Identifier().GetText())
	for _, ext := range ctx.AllTypeExtension() {
		extension := ext.(*parser.TypeExtensionContext)
		if extension.SymbolAsterisk() != nil {
			typ = compiler.GetPointerType(typ)
		}

		if extension.ArrayTypeExtension() != nil {
			extCtx := extension.ArrayTypeExtension().(*parser.ArrayTypeExtensionContext)
			size, err := strconv.Atoi(extCtx.ValueInteger().GetText())
			if err != nil {
				panic(fmt.Sprintf("ERROR: failed to parse array size: %d, %s", size, err.Error()))
			}

			typ = compiler.GetArrayType(typ, size)
		}
	}

	return typ
}

func (v *AstVisitor) VisitTypeAnnotation(ctx *parser.TypeAnnotationContext) any {
	return v.Visit(ctx.TypeName())
}
