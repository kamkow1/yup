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
