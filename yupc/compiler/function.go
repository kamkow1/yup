package compiler

import (
	"log"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

// ----------------------
// User-defined functions
// ----------------------

type FuncParam struct {
	Name string
	Type llvm.Type
}

type Function struct {
	name      string
	value     llvm.Value
	params    []FuncParam
	exitBlock *llvm.BasicBlock
}

func (v *AstVisitor) VisitFunctionParameter(ctx *parser.FunctionParameterContext) any {
	return v.Visit(ctx.TypeAnnotation())
}

func (v *AstVisitor) VisitFunctionParameterList(ctx *parser.FunctionParameterListContext) any {
	var params []FuncParam
	for _, p := range ctx.AllFunctionParameter() {
		params = append(params, FuncParam{
			Name: p.(*parser.FunctionParameterContext).Identifier().GetText(),
			Type: v.Visit(p).(llvm.Type)})
	}

	return params
}

func (v *AstVisitor) VisitFunctionSignature(ctx *parser.FunctionSignatureContext) any {
	name := ctx.Identifier().GetText()
	var params []FuncParam
	if ctx.FunctionParameterList() != nil {
		params = v.Visit(ctx.FunctionParameterList()).([]FuncParam)
	} else {
		params = make([]FuncParam, 0)
	}

	var returnType llvm.Type
	if ctx.TypeName() != nil {
		returnType = v.Visit(ctx.TypeName()).(llvm.Type)
	} else {
		returnType = llvm.VoidType()
	}

	var types []llvm.Type
	for _, fp := range params {
		types = append(types, fp.Type)
	}

	funcType := llvm.FunctionType(returnType, types, false)
	function := llvm.AddFunction(CompilationUnits.Peek().Module, name, funcType)

	for i, pt := range params {
		function.Param(i).SetName(pt.Name)
	}

	if ctx.AttributeList() != nil {
		attrs := v.Visit(ctx.AttributeList()).([]*Attribute)
		for _, a := range attrs {
			switch a.name {
			case "gc":
				{
					gc := a.params[0]
					if gc == "default" {
						function.SetGC("coreclr")
					} else {
						function.SetGC(gc)
					}
				}
			case "link_type":
				{
					linkage := a.params[0]
					function.SetLinkage(GetLinkageFromString(linkage))
				}
			}
		}
	}

	CompilationUnits.Peek().Functions[name] = Function{
		name,
		function,
		params,
		&llvm.BasicBlock{},
	}

	return function
}

func (v *AstVisitor) VisitFunctionDefinition(ctx *parser.FunctionDefinitionContext) any {
	signature := v.Visit(ctx.FunctionSignature()).(llvm.Value)
	isVoid := signature.Type().ReturnType().ElementType().TypeKind() == llvm.VoidTypeKind
	function := CompilationUnits.Peek().Functions[signature.Name()]

	CreateBlock()
	bodyBlock := llvm.AddBasicBlock(signature, "body")
	*function.exitBlock = llvm.AddBasicBlock(signature, "exit")

	CompilationUnits.Peek().Builder.SetInsertPointAtEnd(bodyBlock)

	if !isVoid {
		name := "__return_value"
		returnType := function.value.Type().ReturnType().ElementType()
		a := CompilationUnits.Peek().Builder.CreateAlloca(returnType, name)
		loc := LocalVariable{name, true, a}
		CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][name] = loc
	}

	if signature.ParamsCount() > 0 {
		for _, p := range signature.Params() {
			alloca := CompilationUnits.Peek().Builder.CreateAlloca(p.Type(), "")
			n := len(CompilationUnits.Peek().Locals) - 1
			CompilationUnits.Peek().Locals[n][p.Name()] = LocalVariable{p.Name(), false, alloca}
			CompilationUnits.Peek().Builder.CreateStore(p, alloca)
		}
	}

	v.Visit(ctx.CodeBlock())
	CompilationUnits.Peek().Builder.SetInsertPointAtEnd(*function.exitBlock)

	if isVoid {
		return CompilationUnits.Peek().Builder.CreateRetVoid()
	} else {
		returnValue := FindLocalVariable("__return_value", len(CompilationUnits.Peek().Locals)-1)
		load := CompilationUnits.Peek().Builder.CreateLoad(returnValue.Value, "")
		return CompilationUnits.Peek().Builder.CreateRet(load)
	}
}

func (v *AstVisitor) VisitFunctionCallArgList(ctx *parser.FunctionCallArgListContext) any {
	var args []llvm.Value
	for _, expr := range ctx.AllExpression() {
		args = append(args, v.Visit(expr).(llvm.Value))
	}

	return args
}

func (v *AstVisitor) VisitFunctionCallExpression(ctx *parser.FunctionCallExpressionContext) any {
	return v.Visit(ctx.FunctionCall())
}

func (v *AstVisitor) VisitFunctionCall(ctx *parser.FunctionCallContext) any {
	name := ctx.Identifier().GetText()
	var args []llvm.Value
	if ctx.FunctionCallArgList() != nil {
		args = v.Visit(ctx.FunctionCallArgList()).([]llvm.Value)
	} else {
		args = make([]llvm.Value, 0)
	}

	f := CompilationUnits.Peek().Module.NamedFunction(name)
	if f.IsNil() {
		log.Fatalf("ERROR: tried to call an unknown function: %s", name)
	}

	return CompilationUnits.Peek().Builder.CreateCall(f, args, "")
}

func (v *AstVisitor) VisitFunctionReturn(ctx *parser.FunctionReturnContext) any {
	functionName := CompilationUnits.Peek().Builder.GetInsertBlock().Parent().Name()
	function := CompilationUnits.Peek().Functions[functionName]
	if ctx.Expression() != nil {
		value := v.Visit(ctx.Expression()).(llvm.Value)
		returnValue := FindLocalVariable("__return_value", len(CompilationUnits.Peek().Locals)-1)
		CompilationUnits.Peek().Builder.CreateStore(value, returnValue.Value)
		return CompilationUnits.Peek().Builder.CreateBr(*function.exitBlock)
	}

	return CompilationUnits.Peek().Builder.CreateBr(*function.exitBlock)
}

// -----------------------------
// Compiler's built-in functions
// -----------------------------

type functionType func([]any) llvm.Value

var functions map[string]functionType = map[string]functionType{
	"cast":              Cast,
	"size_of":           SizeOf,
	"type_name_of":      TypeNameOf,
	"is_type_equal":     IsTypeEqual,
	"type_name_of_func": TypeNameOfFunc,
}

func TypeNameOfFunc(args []any) llvm.Value {
	name := args[0].(string)

	if !CompilationUnits.Peek().Module.NamedFunction(name).IsNil() {
		function := CompilationUnits.Peek().Module.NamedFunction(name)
		typ := function.Type().String()

		return CompilationUnits.Peek().Builder.CreateGlobalStringPtr(typ, "")
	} else {
		log.Fatalf("ERROR: unknown function name passed to @TypeNameOfFunc(): %s", name)
		panic("ERROR: TypeNameOfFunc(): unreachable")
	}

	panic("ERROR: TypeNameOfFunc(): unreachable")
}

func IsTypeEqual(args []any) llvm.Value {
	typ0 := args[0].(llvm.Value).Type()
	typ1 := args[1].(llvm.Value).Type()

	var result llvm.Value
	if typ0.TypeKind() == typ1.TypeKind() {
		result = llvm.ConstInt(llvm.Int1Type(), 1, false)
	} else {
		result = llvm.ConstInt(llvm.Int1Type(), 0, false)
	}

	return result
}

func Cast(args []any) llvm.Value {
	value := args[0].(llvm.Value)
	typ := args[1].(llvm.Type)

	if value.Type().TypeKind() == llvm.IntegerTypeKind && typ.TypeKind() == llvm.IntegerTypeKind {
		return CompilationUnits.Peek().Builder.CreateIntCast(value, typ, "")
	} else if value.Type().TypeKind() == llvm.IntegerTypeKind && typ.TypeKind() == llvm.PointerTypeKind {
		return CompilationUnits.Peek().Builder.CreateIntToPtr(value, typ, "")
	} else if value.Type().TypeKind() == llvm.PointerTypeKind && typ.TypeKind() == llvm.IntegerTypeKind {
		return CompilationUnits.Peek().Builder.CreatePtrToInt(value, typ, "")
	} else if value.Type().TypeKind() == llvm.PointerTypeKind && typ.TypeKind() == llvm.PointerTypeKind {
		return CompilationUnits.Peek().Builder.CreatePointerCast(value, typ, "")
	} else {
		return CompilationUnits.Peek().Builder.CreateBitCast(value, typ, "")
	}
}

func SizeOf(args []any) llvm.Value {

	var result llvm.Value
	switch t := args[0].(type) {
	case llvm.Type:
		result = llvm.SizeOf(t)
	case llvm.Value:
		result = llvm.SizeOf(t.Type())
	default:
		log.Fatal("ERROR: @SizeOf(): unknown value in function")
	}

	return result
}

func TypeNameOf(args []any) llvm.Value {
	val := args[0].(llvm.Value)
	typeName := val.Type().String()

	return CompilationUnits.Peek().Builder.CreateGlobalStringPtr(typeName, "")
}

func (v *AstVisitor) VisitYupFunctionExpression(ctx *parser.YupFunctionExpressionContext) any {
	return v.Visit(ctx.YupFunction())
}

func (v *AstVisitor) VisitYupFunction(ctx *parser.YupFunctionContext) any {
	callCtx := ctx.FunctionCall().(*parser.FunctionCallContext)
	name := callCtx.Identifier().GetText()
	var arguments []any
	args := callCtx.FunctionCallArgList().(*parser.FunctionCallArgListContext)
	for _, a := range args.AllExpression() {
		arguments = append(arguments, v.Visit(a))
	}

	if f, ok := functions[name]; !ok {
		log.Fatalf("ERROR: tried to call an unknown built-in function: %s", name)
		return nil
	} else {
		return f(arguments)
	}
}
