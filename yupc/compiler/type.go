package compiler

import (
	"strconv"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

type TypeInfo struct {
	Name     string
	Type     llvm.Type
	IsPublic bool
}

func InitTypeMap() map[string]*TypeInfo {
	return map[string]*TypeInfo{
		"i1": &TypeInfo{
			Name:     "i1",
			Type:     llvm.Int1Type(),
			IsPublic: true,
		},
		"i8": &TypeInfo{
			Name:     "i8",
			Type:     llvm.Int8Type(),
			IsPublic: true,
		},
		"i16": &TypeInfo{
			Name:     "i16",
			Type:     llvm.Int16Type(),
			IsPublic: true,
		},
		"i32": &TypeInfo{
			Name:     "i32",
			Type:     llvm.Int32Type(),
			IsPublic: true,
		},
		"i64": &TypeInfo{
			Name:     "i64",
			Type:     llvm.Int64Type(),
			IsPublic: true,
		},
		"f32": &TypeInfo{
			Name:     "f32",
			Type:     llvm.FloatType(),
			IsPublic: true,
		},
		"f64": &TypeInfo{
			Name:     "f64",
			Type:     llvm.DoubleType(),
			IsPublic: true,
		},
		"f128": &TypeInfo{
			Name:     "f128",
			Type:     llvm.FP128Type(),
			IsPublic: true,
		},
		"x64fp80": &TypeInfo{
			Name:     "x64fp80",
			Type:     llvm.X86FP80Type(),
			IsPublic: true,
		},
		"voidbyte": &TypeInfo{
			Name:     "voidbyte",
			Type:     llvm.Int8Type(),
			IsPublic: true,
		},
		"void": &TypeInfo{
			Name:     "void",
			Type:     llvm.VoidType(),
			IsPublic: true,
		},
	}
}

func GetTypeFromName(name string) *TypeInfo {
	typ, ok := CompilationUnits.Peek().Types[name]
	if !ok {
		LogError("unknown type: %s", name)
	}

	return typ
}

func reverseTypeExtList(array []parser.ITypeExtContext) []parser.ITypeExtContext {
	if len(array) == 0 {
		return array
	}

	return append(reverseTypeExtList(array[1:]), array[0])
}

func (v *AstVisitor) VisitTypeName(ctx *parser.TypeNameContext) any {

	var typ *TypeInfo
	if ctx.Identifier() != nil {
		typ = GetTypeFromName(ctx.Identifier().GetText())
	} else if ctx.StructType() != nil {
		strtp := ctx.StructType().(*parser.StructTypeContext)
		var types []llvm.Type
		for _, tp := range strtp.AllTypeName() {
			types = append(types, v.Visit(tp).(*TypeInfo).Type)
		}

		typ = &TypeInfo{
			Type: llvm.StructType(types, false),
		}
	} else if ctx.FuncType() != nil {
		fntp := ctx.FuncType().(*parser.FuncTypeContext)

		var retType llvm.Type
		if fntp.SymbolArrow() != nil {
			retType = v.Visit(fntp.TypeName()).(llvm.Type)
		} else {
			retType = llvm.VoidType()
		}

		params := make([]FuncParam, 0)
		if fntp.FuncParamList() != nil {
			params = v.Visit(fntp.FuncParamList()).([]FuncParam)
		}

		paramTypes := make([]llvm.Type, 0)
		isva := false
		for _, pt := range params {
			if pt.IsVarArg {
				isva = true
				break
			}

			paramTypes = append(paramTypes, pt.Type.Type)
		}

		typ = &TypeInfo{
			Type: llvm.FunctionType(retType, paramTypes, isva),
		}
	}

	for _, ext := range reverseTypeExtList(ctx.AllTypeExt()) {
		extension := ext.(*parser.TypeExtContext)
		if extension.SymbolAsterisk() != nil {
			typ = &TypeInfo{
				Type: llvm.PointerType(typ.Type, 0),
			}
		}

		if extension.ArrayTypeExt() != nil {
			extctx := extension.ArrayTypeExt().(*parser.ArrayTypeExtContext)
			size, _ := strconv.Atoi(extctx.ValueInteger().GetText())
			typ = &TypeInfo{
				Type: llvm.ArrayType(typ.Type, size),
			}
		}
	}

	return typ
}

type Field struct {
	Name string
	Type *TypeInfo
}

type Structure struct {
	Name     string
	Fields   []*Field
	Type     *TypeInfo
	IsPublic bool
}

func (v *AstVisitor) VisitStructField(ctx *parser.StructFieldContext) any {
	return &Field{
		Name: ctx.Identifier().GetText(),
		Type: v.Visit(ctx.TypeAnnot()).(*TypeInfo),
	}
}

func (v *AstVisitor) VisitStructDeclaration(ctx *parser.StructDeclarationContext) any {
	name := ctx.Identifier().GetText()
	ispub := ctx.KeywordPublic() != nil
	c := CompilationUnits.Peek().Module.Context()
	structType := c.StructCreateNamed(name)
	CompilationUnits.Peek().Types[name] = &TypeInfo{
		Name:     name,
		Type:     structType,
		IsPublic: ispub,
	}

	var fields []*Field
	for _, fld := range ctx.AllStructField() {
		fields = append(fields, v.Visit(fld).(*Field))
	}

	var fieldTypes []llvm.Type
	for _, fld := range fields {
		fieldTypes = append(fieldTypes, fld.Type.Type)
	}

	structType.StructSetBody(fieldTypes, false)

	strct := Structure{
		Name:     name,
		Fields:   fields,
		IsPublic: ispub,
		Type: &TypeInfo{
			Name: name,
			Type: structType,
		},
	}

	CompilationUnits.Peek().Structs[name] = strct
	return structType
}

func (v *AstVisitor) VisitTypeAliasDeclaration(ctx *parser.TypeAliasDeclarationContext) any {
	original := v.Visit(ctx.TypeName()).(llvm.Type)
	name := ctx.Identifier().GetText()

	CompilationUnits.Peek().Types[name] = &TypeInfo{
		Name: name,
		Type: original,
	}

	return nil
}

func (v *AstVisitor) VisitFieldAccessExpression(ctx *parser.FieldAccessExpressionContext) any {
	strct := v.Visit(ctx.Expression()).(llvm.Value)
	fieldName := ctx.Identifier().GetText()

	return GetStructFieldPtr(strct, fieldName)
}

func (v *AstVisitor) VisitMethodCallExpression(ctx *parser.MethodCallExpressionContext) any {
	strct := v.Visit(ctx.Expression()).(llvm.Value)
	fncctx := ctx.FuncCall().(*parser.FuncCallContext)
	name := fncctx.Identifier().GetText()

	method := GetStructFieldPtr(strct, name)
	method = CompilationUnits.Peek().Builder.CreateLoad(method.Type().ElementType(), method, "")
	args := v.Visit(fncctx.FuncCallArgList()).([]llvm.Value)
	return CompilationUnits.Peek().Builder.CreateCall(method.Type().ReturnType(), method, args, "")
}

func (v *AstVisitor) VisitStructInitExpression(ctx *parser.StructInitExpressionContext) any {
	return v.Visit(ctx.StructInit())
}

func (v *AstVisitor) VisitStructInit(ctx *parser.StructInitContext) any {
	name := ctx.Identifier().GetText()
	strct, ok := CompilationUnits.Peek().Types[name]
	if !ok {
		LogError("tried to initialize an unknown struct type: `%s`", name)
	}

	var vals []llvm.Value
	for i, expr := range ctx.AllExpression() {
		arg := v.Visit(expr).(llvm.Value)
		voidptr := llvm.PointerType(llvm.Int8Type(), 0)
		isParamVoidPtr := strct.Fields[i].Type.Type == voidptr
		isArgVoidPtr := arg.Type() == voidptr

		if isParamVoidPtr && !isArgVoidPtr {
			arg = Cast(arg, voidptr)
		}

		vals = append(vals, arg)
	}

	return llvm.ConstNamedStruct(strct.Type, vals)
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

func GetStructFieldPtr(strct llvm.Value, fieldname string) llvm.Value {
	if strct.Type().TypeKind() != llvm.PointerTypeKind {
		LogError("cannot access struct fields on a non-pointer type: `%s`", strct.Type().String())
	}

	strctname := strct.Type().ElementType().StructName()
	baseStruct := CompilationUnits.Peek().Structs[strctname]

	var field llvm.Value
	for i, f := range baseStruct.Fields {
		if fieldname == f.Name {
			field = CompilationUnits.Peek().Builder.CreateStructGEP(strct.Type().ElementType(), strct, i, "")
		}
	}

	return field
}
