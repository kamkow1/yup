package compiler

import (
	"strconv"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

func InitTypeMap() map[string]llvm.Type {
	return map[string]llvm.Type{
		"i1":      llvm.Int1Type(),
		"i8":      llvm.Int8Type(),
		"i16":     llvm.Int16Type(),
		"i32":     llvm.Int32Type(),
		"i64":     llvm.Int64Type(),
		"f32":     llvm.FloatType(),
		"f64":     llvm.DoubleType(),
		"f128":    llvm.FP128Type(),
		"x64fp80": llvm.X86FP80Type(),
		"void":    llvm.VoidType(),
	}
}

func GetTypeFromName(name string) llvm.Type {
	typ, ok := CompilationUnits.Peek().Types[name]
	if !ok {
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

func (v *AstVisitor) VisitLiteralTypeExpression(ctx *parser.LiteralTypeExpressionContext) any {
	return v.Visit(ctx.TypeName())
}

func (v *AstVisitor) VisitTypeName(ctx *parser.TypeNameContext) any {

	var typ llvm.Type
	if ctx.Identifier() != nil {
		typ = GetTypeFromName(ctx.Identifier().GetText())
	} else if ctx.StructType() != nil {
		strtp := ctx.StructType().(*parser.StructTypeContext)
		var types []llvm.Type
		for _, tp := range strtp.AllTypeName() {
			types = append(types, v.Visit(tp).(llvm.Type))
		}

		typ = llvm.StructType(types, false)
	} else if ctx.FunctionType() != nil {
		fntp := ctx.FunctionType().(*parser.FunctionTypeContext)

		var retType llvm.Type
		if fntp.SymbolArrow() != nil {
			retType = v.Visit(fntp.TypeName()).(llvm.Type)
		} else {
			retType = llvm.VoidType()
		}

		params := make([]FuncParam, 0)
		if fntp.FunctionParameterList() != nil {
			params = v.Visit(fntp.FunctionParameterList()).([]FuncParam)
		}

		paramTypes := make([]llvm.Type, 0)
		isva := false
		for _, pt := range params {
			if pt.IsVarArg {
				isva = true
				break
			}

			paramTypes = append(paramTypes, pt.Type)
		}

		typ = llvm.FunctionType(retType, paramTypes, isva)
	}

	for _, ext := range reverseTypeExtList(ctx.AllTypeExtension()) {
		extension := ext.(*parser.TypeExtensionContext)
		if extension.SymbolAsterisk() != nil {
			typ = llvm.PointerType(typ, 0)
		}

		if extension.ArrayTypeExtension() != nil {
			extctx := extension.ArrayTypeExtension().(*parser.ArrayTypeExtensionContext)
			size, _ := strconv.Atoi(extctx.ValueInteger().GetText())
			typ = llvm.ArrayType(typ, size)
		}
	}

	return typ
}

func (v *AstVisitor) VisitTypeAnnotation(ctx *parser.TypeAnnotationContext) any {
	return v.Visit(ctx.TypeName())
}

type GenericParam struct {
	Name     string
	InstType llvm.Type
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

	if ctx.GenericParams() == nil {
		c := CompilationUnits.Peek().Module.Context()
		structType := c.StructCreateNamed(name)
		CompilationUnits.Peek().Types[name] = structType

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
	} else {
		// TODO: implement generic struct types
		return nil
	}
}

func (v *AstVisitor) VisitTypeAliasDeclaration(ctx *parser.TypeAliasDeclarationContext) any {
	ogType := v.Visit(ctx.TypeName()).(llvm.Type)
	name := ctx.Identifier().GetText()

	CompilationUnits.Peek().Types[name] = ogType

	return nil
}

func (v *AstVisitor) VisitFieldAccessExpression(ctx *parser.FieldAccessExpressionContext) any {
	strct := v.Visit(ctx.Expression()).(llvm.Value)
	fieldName := ctx.Identifier().GetText()

	return GetStructFieldPtr(strct, fieldName)
}

func (v *AstVisitor) VisitMethodCallExpression(ctx *parser.MethodCallExpressionContext) any {
	strct := v.Visit(ctx.Expression()).(llvm.Value)
	fncctx := ctx.FunctionCall().(*parser.FunctionCallContext)
	name := fncctx.Identifier().GetText()

	method := GetStructFieldPtr(strct, name)
	method = CompilationUnits.Peek().Builder.CreateLoad(method.Type().ElementType(), method, "")
	args := v.Visit(fncctx.FunctionCallArgList()).([]llvm.Value)
	return CompilationUnits.Peek().Builder.CreateCall(method.Type().ReturnType(), method, args, "")
}

func (v *AstVisitor) VisitStructInitExpression(ctx *parser.StructInitExpressionContext) any {
	return v.Visit(ctx.StructInit())
}

func (v *AstVisitor) VisitStructInit(ctx *parser.StructInitContext) any {
	name := ctx.Identifier().GetText()
	strct := CompilationUnits.Peek().Types[name]

	var vals []llvm.Value
	for _, expr := range ctx.AllExpression() {
		vals = append(vals, v.Visit(expr).(llvm.Value))
	}

	return llvm.ConstNamedStruct(strct, vals)
}

func (v *AstVisitor) VisitConstStructInitExpression(ctx *parser.ConstStructInitExpressionContext) any {
	return v.Visit(ctx.ConstStructInit())
}

func (v *AstVisitor) VisitConstStructInit(ctx *parser.ConstStructInitContext) any {
	var values []llvm.Value
	for _, expr := range ctx.AllExpression() {
		values = append(values, v.Visit(expr).(llvm.Value))
	}

	return llvm.ConstStruct(values, false)
}
