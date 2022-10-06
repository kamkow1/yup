package compiler

import (
	"strconv"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

var Types map[string]llvm.Type = map[string]llvm.Type{
	"i1":   llvm.Int1Type(),
	"i8":   llvm.Int8Type(),
	"i16":  llvm.Int16Type(),
	"i32":  llvm.Int32Type(),
	"i64":  llvm.Int64Type(),
	"f32":  llvm.FloatType(),
	"f128": llvm.FP128Type(),
	"void": llvm.VoidType(),
}

func GetPointerType(typ llvm.Type) llvm.Type {
	return llvm.PointerType(typ, 0)
}

func GetArrayType(typ llvm.Type, count int) llvm.Type {
	return llvm.ArrayType(typ, count)
}

func GetTypeFromName(name string) llvm.Type {
	var typ llvm.Type
	if llvmType, ok := Types[name]; ok {
		typ = llvmType
	} else {
		LogError("unknown type: %s", name)
	}

	return typ
}

func AssertType(typ1 llvm.Type, typ2 llvm.Type) bool {
	return typ1.TypeKind() != typ2.TypeKind()
}

func reverseTypeExtList(array []parser.ITypeExtensionContext) []parser.ITypeExtensionContext {
	if len(array) == 0 {
		return array
	}

	return append(reverseTypeExtList(array[1:]), array[0])
}

func (v *AstVisitor) VisitTypeName(ctx *parser.TypeNameContext) any {
	typ := GetTypeFromName(ctx.Identifier().GetText())
	for _, ext := range reverseTypeExtList(ctx.AllTypeExtension()) {
		extension := ext.(*parser.TypeExtensionContext)
		if extension.SymbolAsterisk() != nil {
			typ = GetPointerType(typ)
		}

		if extension.ArrayTypeExtension() != nil {
			extCtx := extension.ArrayTypeExtension().(*parser.ArrayTypeExtensionContext)
			size, err := strconv.Atoi(extCtx.ValueInteger().GetText())
			if err != nil {
				LogError("failed to parse array size: %d, %s", size, err.Error())
			}

			typ = GetArrayType(typ, size)
		}
	}

	return typ
}

func (v *AstVisitor) VisitTypeAnnotation(ctx *parser.TypeAnnotationContext) any {
	return v.Visit(ctx.TypeName())
}

func (v *AstVisitor) VisitTypeExpression(ctx *parser.TypeExpressionContext) any {
	return v.Visit(ctx.TypeNameExpression())
}

func (v *AstVisitor) VisitTypeNameExpression(ctx *parser.TypeNameExpressionContext) any {
	return v.Visit(ctx.TypeName())
}

type Field struct {
	Name string
	Type llvm.Type
}

type Structure struct {
	Name   string
	Fields []Field
}

func (v *AstVisitor) VisitStructField(ctx *parser.StructFieldContext) any {
	return Field{
		Name: ctx.Identifier().GetText(),
		Type: v.Visit(ctx.TypeAnnotation()).(llvm.Type),
	}
}

func (v *AstVisitor) VisitStructDeclaration(ctx *parser.StructDeclarationContext) any {
	name := ctx.Identifier().GetText()
	c := CompilationUnits.Peek().Module.Context()
	structType := c.StructCreateNamed(name)
	Types[name] = structType

	var fields []Field
	for _, fld := range ctx.AllStructField() {
		fields = append(fields, v.Visit(fld).(Field))
	}

	strct := Structure{
		Name:   name,
		Fields: fields,
	}

	CompilationUnits.Peek().Structs[name] = strct

	var fieldTypes []llvm.Type
	for _, fld := range fields {
		fieldTypes = append(fieldTypes, fld.Type)
	}

	structType.StructSetBody(fieldTypes, false)

	return structType
}

func (v *AstVisitor) VisitTypeAliasDeclaration(ctx *parser.TypeAliasDeclarationContext) any {
	ogType := v.Visit(ctx.TypeName()).(llvm.Type)
	name := ctx.Identifier().GetText()

	Types[name] = ogType

	return nil
}

func (v *AstVisitor) VisitFieldAccessExpression(ctx *parser.FieldAccessExpressionContext) any {
	strct := v.Visit(ctx.Expression()).(llvm.Value)
	isptr := strct.Type().TypeKind() == llvm.PointerTypeKind

	if isptr {
		strct = CompilationUnits.Peek().Builder.CreateLoad(strct, "")
	}

	var name string
	if isptr {
		name = strct.Type().ElementType().StructName()
	} else {
		name = strct.Type().StructName()
	}

	fieldName := ctx.Identifier().GetText()
	baseStruct, _ := CompilationUnits.Peek().Structs[name]

	var field llvm.Value
	for i, f := range baseStruct.Fields {
		if fieldName == f.Name {
			field = CompilationUnits.Peek().Builder.CreateStructGEP(strct, i, "")
			//field = CompilationUnits.Peek().Builder.CreateLoad(gep, "")
		}
	}

	return field
}
