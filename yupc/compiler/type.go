package compiler

import (
	"strconv"
	"strings"

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

func GetPointerType(typ llvm.Type) llvm.Type {
	return llvm.PointerType(typ, 0)
}

func GetArrayType(typ llvm.Type, count int) llvm.Type {
	return llvm.ArrayType(typ, count)
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
	} else {
		strtp := ctx.StructType().(*parser.StructTypeContext)
		var types []llvm.Type
		for _, tp := range strtp.AllTypeName() {
			types = append(types, v.Visit(tp).(llvm.Type))
		}

		typ = llvm.StructType(types, false)
	}

	return typ
}

func (v *AstVisitor) VisitTypeAnnotation(ctx *parser.TypeAnnotationContext) any {
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

	if ctx.AttributeList() != nil {
		attrs := v.Visit(ctx.AttributeList()).([]*Attribute)
		for _, attr := range attrs {
			switch attr.Name {
			case "auto_new":
				{
					fncname := "new_" + strings.ToLower(name)
					var params []FuncParam
					for _, p := range fields {
						paramName := "_" + p.Name
						fp := FuncParam{
							Name:     paramName,
							IsVarArg: false,
							IsConst:  true,
							Type:     p.Type,
						}

						params = append(params, fp)
					}

					var paramTypes []llvm.Type
					for _, p := range params {
						paramTypes = append(paramTypes, p.Type)
					}

					rtType := GetPointerType(structType)
					ft := llvm.FunctionType(rtType, paramTypes, false)
					function := llvm.AddFunction(CompilationUnits.Peek().Module, fncname, ft)

					CompilationUnits.Peek().Functions[fncname] = Function{
						Name:      fncname,
						Value:     &function,
						Params:    params,
						ExitBlock: &llvm.BasicBlock{},
					}

					bodyblock := llvm.AddBasicBlock(function, "gen_body")
					CompilationUnits.Peek().Builder.SetInsertPointAtEnd(bodyblock)

					structValue := CompilationUnits.Peek().Builder.CreateAlloca(rtType, "")
					mallocCall := CompilationUnits.Peek().Builder.CreateMalloc(structType, "")
					CompilationUnits.Peek().Builder.CreateStore(mallocCall, structValue)

					var paramAllocas []llvm.Value
					for i, p := range paramTypes {
						alloca := CompilationUnits.Peek().Builder.CreateAlloca(p, "")
						CompilationUnits.Peek().Builder.CreateStore(function.Param(i), alloca)

						paramAllocas = append(paramAllocas, alloca)
					}

					for i, _ := range fields {
						ld := CompilationUnits.Peek().Builder.CreateLoad(structValue, "")
						field := CompilationUnits.Peek().Builder.CreateStructGEP(ld, i, "")
						allocald := CompilationUnits.Peek().Builder.CreateLoad(paramAllocas[i], "")
						CompilationUnits.Peek().Builder.CreateStore(allocald, field)
					}

					ld := CompilationUnits.Peek().Builder.CreateLoad(structValue, "")
					CompilationUnits.Peek().Builder.CreateRet(ld)
				}
			case "auto_free":
				{
					fncname := "free_" + strings.ToLower(name)
					var params []FuncParam
					paramName := "_" + strings.ToLower(name)
					paramType := GetPointerType(structType)
					p := FuncParam{
						Name:     paramName,
						IsVarArg: false,
						IsConst:  true,
						Type:     paramType,
					}

					params = append(params, p)

					var paramTypes []llvm.Type
					paramTypes = append(paramTypes, paramType)

					ft := llvm.FunctionType(llvm.VoidType(), paramTypes, false)
					function := llvm.AddFunction(CompilationUnits.Peek().Module, fncname, ft)

					CompilationUnits.Peek().Functions[fncname] = Function{
						Name:      fncname,
						Value:     &function,
						Params:    params,
						ExitBlock: &llvm.BasicBlock{},
					}

					bodyblock := llvm.AddBasicBlock(function, "gen_body")
					CompilationUnits.Peek().Builder.SetInsertPointAtEnd(bodyblock)

					paramAlloca := CompilationUnits.Peek().Builder.CreateAlloca(paramType, "")
					CompilationUnits.Peek().Builder.CreateStore(function.Param(0), paramAlloca)

					ld := CompilationUnits.Peek().Builder.CreateLoad(paramAlloca, "")
					CompilationUnits.Peek().Builder.CreateFree(ld)
					CompilationUnits.Peek().Builder.CreateRetVoid()
				}
			}
		}
	}

	return structType
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

	return GetStructFieldPtr(strct, fieldName, false)
}

func (v *AstVisitor) VisitStructInitExpression(ctx *parser.StructInitExpressionContext) any {
	return v.Visit(ctx.StructInit())
}

func (v *AstVisitor) VisitFieldInit(ctx *parser.FieldInitContext) any {
	return v.Visit(ctx.VariableValue())
}

func (v *AstVisitor) VisitStructInit(ctx *parser.StructInitContext) any {
	name := ctx.Identifier().GetText()
	strct := CompilationUnits.Peek().Types[name]
	structBase := CompilationUnits.Peek().Structs[name]

	// malloc := CompilationUnits.Peek().Builder.CreateMalloc(strct, "")
	mallocfn := CompilationUnits.Peek().Module.NamedFunction("malloc")
	if mallocfn.IsNil() {
		pts := []llvm.Type{llvm.Int64Type()}
		ft := llvm.FunctionType(llvm.PointerType(llvm.Int8Type(), 0), pts, false)
		mallocfn = llvm.AddFunction(CompilationUnits.Peek().Module, "malloc", ft)
	}

	args := []llvm.Value{llvm.SizeOf(strct)}
	malloc := CompilationUnits.Peek().Builder.CreateCall(mallocfn, args, "")
	allocatedStruct := Cast(malloc, llvm.PointerType(strct, 0))

	for i, fld := range structBase.Fields {
		fieldptr := GetStructFieldPtr(allocatedStruct, fld.Name, true)
		init := v.Visit(ctx.FieldInit(i)).(llvm.Value)
		CompilationUnits.Peek().Builder.CreateStore(init, fieldptr)
	}

	return allocatedStruct
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
