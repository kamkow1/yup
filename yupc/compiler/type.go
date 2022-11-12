package compiler

import (
	"strconv"
	"strings"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

type TypeInfo struct {
	Name     string
	Type     llvm.Type
	IsPublic bool
}

// initialize built-in types
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
		"void": &TypeInfo{
			Name:     "void",
			Type:     llvm.Int8Type(),
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
	Methods  []*Function
	Type     *TypeInfo
	IsPublic bool
}

func (v *AstVisitor) VisitStructField(ctx *parser.StructFieldContext) any {
	return &Field{
		Name: ctx.Identifier().GetText(),
		Type: v.Visit(ctx.TypeAnnot()).(*TypeInfo),
	}
}

var StructNameStack = NewStack[string]()

func (v *AstVisitor) VisitStructDeclaration(ctx *parser.StructDeclarationContext) any {
	name := ctx.Identifier(0).GetText()
	StructNameStack.Push(&name)

	ispub := ctx.KeywordPublic() != nil
	c := CompilationUnits.Peek().Module.Context()
	structType := c.StructCreateNamed(name)
	CompilationUnits.Peek().Types[name] = &TypeInfo{
		Name:     name,
		Type:     structType,
		IsPublic: ispub,
	}

	fields := make([]*Field, 0)
	methods := make([]*Function, 0)

	// inherit other structs
	if ctx.SymbolExclMark() != nil { // has inherit list
		for i, inh := range ctx.AllIdentifier() {
			if i != 0 {
				structName := inh.GetText()
				strct := CompilationUnits.Peek().Structs[structName]
				for _, field := range strct.Fields {
					fields = append(fields, field)
				}

				for _, method := range strct.Methods {
					// construct a new method for the struct
					split := strings.Split(method.Value.Name(), "_")
					split = split[1:len(split)]
					newName := name + "_" + strings.Join(split, "_")

					returnType := method.Value.Type().ElementType().ReturnType()
					paramTypes := method.Value.Type().ParamTypes()
					vararg := method.Value.Type().IsFunctionVarArg()

					functionType := llvm.FunctionType(returnType, paramTypes, vararg)

					module := CompilationUnits.Peek().Module
					functionValue := llvm.AddFunction(*module, newName, functionType)

					newMethod := &Function{
						Name:           newName,
						Value:          &functionValue,
						ExitBlock:      method.ExitBlock,
						MethodName:     method.MethodName,
						IsPublicMethod: method.IsPublicMethod,
						HasSelf:        method.HasSelf,
					}

					methods = append(methods, newMethod)
				}
			}
		}
	}

	if ctx.SymbolLbrace() != nil { // struct has a body
		// emit struct fields
		for _, field := range ctx.AllStructField() {
			fields = append(fields, v.Visit(field).(*Field))
		}

		var fieldTypes []llvm.Type
		for _, field := range fields {
			fieldTypes = append(fieldTypes, field.Type.Type)
		}

		structType.StructSetBody(fieldTypes, false)
		CompilationUnits.Peek().Types[name].Type = structType
		CompilationUnits.Peek().Structs[name] = &Structure{
			Name:     name,
			Fields:   fields,
			Methods:  []*Function{},
			IsPublic: ispub,
			Type: &TypeInfo{
				Name: name,
				Type: structType,
			},
		}

		// emit struct methods and rename them
		for _, method := range ctx.AllFuncDef() {
			funct := v.Visit(method).(*Function)
			funcName := funct.Name
			newName := name + "_" + funcName
			funct.Name = newName
			funct.Value.SetName(newName)
			funct.Value.SetLinkage(llvm.LinkOnceODRLinkage)
			funct.MethodName = funcName

			// replace key in `Functions` map
			delete(CompilationUnits.Peek().Functions, funcName)
			CompilationUnits.Peek().Functions[newName] = funct

			methods = append(methods, funct)
		}

		CompilationUnits.Peek().Structs[name].Methods = methods
	}

	StructNameStack.Pop()
	return structType
}

func (v *AstVisitor) VisitTypeAliasDeclaration(ctx *parser.TypeAliasDeclarationContext) any {
	original := v.Visit(ctx.TypeName()).(*TypeInfo)
	name := ctx.Identifier().GetText()

	CompilationUnits.Peek().Types[name] = &TypeInfo{
		IsPublic: ctx.KeywordPublic() != nil,
		Name:     name,
		Type:     original.Type,
	}

	return nil
}

func (v *AstVisitor) VisitFieldAccessExpression(ctx *parser.FieldAccessExpressionContext) any {
	strct := v.Visit(ctx.Expression()).(llvm.Value)
	fieldName := ctx.Identifier().GetText()

	return GetStructFieldPtr(strct, fieldName)
}

func FindMethod(methodName, structName string) (llvm.Value, bool) {
	method := CompilationUnits.Peek().Module.NamedFunction(methodName)
	if method.IsNil() {
		LogError("cannot call method `%s` because it doesn't exist on struct `%s` and is not available in the module",
			methodName, structName)
	}

	strct, ok := CompilationUnits.Peek().Structs[structName]
	if !ok {
		LogError("struct type not found { FindMethod() }. struct name: `%s`", structName)
	}

	var foundMethod *Function
	found := false
	for _, method := range strct.Methods {
		if method.Name == methodName {
			foundMethod = method
			found = true
		}
	}

	if !found {
		LogError("could not find method `%s` on struct `%s`", methodName, structName)
	}

	return method, foundMethod.HasSelf
}

func (v *AstVisitor) VisitStaticMethodCallExpr(ctx *parser.StaticMethodCallExprContext) any {
	name := ctx.Identifier().GetText()
	base, ok := CompilationUnits.Peek().Structs[name]

	if !ok {
		LogError("cannot call a static method on struct `%s` because this struct doesn't exist", name)
	}

	fncctx := ctx.FuncCall().(*parser.FuncCallContext)
	methodName := base.Name + "_" + fncctx.Identifier().GetText()

	method, hasToPassSelf := FindMethod(methodName, name)

	if hasToPassSelf {
		LogError("cannot pass self in a static method call")
	}

	args := make([]any, 0)
	if fncctx.FuncCallArgList() != nil {
		args = v.Visit(fncctx.FuncCallArgList()).([]any)
	}

	valueArgs := make([]llvm.Value, 0)
	for _, arg := range args {
		switch arg.(type) {
		case llvm.Type:
		case llvm.Value:
			valueArgs = append(valueArgs, arg.(llvm.Value))
		}
	}

	return CompilationUnits.Peek().Builder.CreateCall(method.Type().ReturnType(), method, valueArgs, "")
}

func (v *AstVisitor) VisitMethodCallExpr(ctx *parser.MethodCallExprContext) any {
	strct := v.Visit(ctx.Expression()).(llvm.Value)
	if strct.Type().TypeKind() != llvm.PointerTypeKind {
		LogError("cannot call methods on non-pointer type expressions: `%s`", strct.Type().String())
	}

	//structName := strings.Split(strct.Type().ElementType().StructName(), ".")[0]
	structName := ctx.Identifier().GetText()
	fncctx := ctx.FuncCall().(*parser.FuncCallContext)
	name := structName + "_" + fncctx.Identifier().GetText()

	method, hasToPassSelf := FindMethod(name, structName)

	args := make([]any, 0)
	if fncctx.FuncCallArgList() != nil {
		args = v.Visit(fncctx.FuncCallArgList()).([]any)
	}

	var valueArgs []llvm.Value
	if hasToPassSelf {
		valueArgs = append(valueArgs, strct)
	}

	for _, arg := range args {
		switch arg.(type) {
		case llvm.Type:
		case llvm.Value:
			valueArgs = append(valueArgs, arg.(llvm.Value))
		}
	}

	return CompilationUnits.Peek().Builder.CreateCall(method.Type().ReturnType(), method, valueArgs, "")
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
		base := CompilationUnits.Peek().Structs[name]
		isParamVoidPtr := base.Fields[i].Type.Type == voidptr
		isArgVoidPtr := arg.Type() == voidptr

		if isParamVoidPtr && !isArgVoidPtr {
			arg = Cast(arg, &TypeInfo{
				Type: voidptr,
			})
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

	strctname := strings.Split(strct.Type().ElementType().StructName(), ".")[0]
	baseStruct, ok := CompilationUnits.Peek().Structs[strctname]
	if !ok {
		LogError("unable to find struct base `%s`", strctname)
	}

	var field llvm.Value
	found := false
	for i, f := range baseStruct.Fields {
		if fieldname == f.Name {
			found = true
			field = CompilationUnits.Peek().Builder.CreateStructGEP(strct.Type().ElementType(), strct, i, "")
		}
	}

	if !found {
		LogError("unable to find field named `%s` on struct `%s`", fieldname, strctname)
	}

	return field
}

func Cast(value llvm.Value, typ *TypeInfo) llvm.Value {
	valtk := value.Type().TypeKind()
	typtk := typ.Type.TypeKind()
	inttk := llvm.IntegerTypeKind
	ptrtk := llvm.PointerTypeKind

	if valtk == inttk && typtk == inttk {
		return CompilationUnits.Peek().Builder.CreateIntCast(value, typ.Type, "")
	}

	if valtk == inttk && typtk == ptrtk {
		return CompilationUnits.Peek().Builder.CreateIntToPtr(value, typ.Type, "")
	}

	if valtk == ptrtk && typtk == inttk {
		return CompilationUnits.Peek().Builder.CreatePtrToInt(value, typ.Type, "")
	}

	if valtk == ptrtk && typtk == ptrtk {
		return CompilationUnits.Peek().Builder.CreatePointerCast(value, typ.Type, "")
	}

	return CompilationUnits.Peek().Builder.CreateBitCast(value, typ.Type, "")
}
