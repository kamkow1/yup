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

	returnType := v.Visit(ctx.TypeName()).(llvm.Type)
	var types []llvm.Type
	for _, fp := range params {
		types = append(types, fp.Type)
	}

	funcType := llvm.FunctionType(returnType, types, false)
	function := llvm.AddFunction(CompilationUnits.Peek().Module, name, funcType)

	for i, pt := range params {
		function.Param(i).SetName(pt.Name)
	}

	return function
}

func (v *AstVisitor) VisitFunctionDefinition(ctx *parser.FunctionDefinitionContext) any {
	signature := v.Visit(ctx.FunctionSignature()).(llvm.Value)
	CreateBlock()
	block := llvm.AddBasicBlock(signature, "entry")
	CompilationUnits.Peek().Builder.SetInsertPoint(block, block.LastInstruction())

	if signature.ParamsCount() > 0 {
		for _, p := range signature.Params() {
			alloca := CompilationUnits.Peek().Builder.CreateAlloca(p.Type(), "")
			n := len(CompilationUnits.Peek().Locals) - 1
			CompilationUnits.Peek().Locals[n][p.Name()] = LocalVariable{p.Name(), false, alloca}
			CompilationUnits.Peek().Builder.CreateStore(p, alloca)
		}
	}

	v.Visit(ctx.CodeBlock())
	if signature.Type().ReturnType().ElementType().TypeKind() == llvm.VoidTypeKind {
		return CompilationUnits.Peek().Builder.CreateRetVoid()
	}

	return nil
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
	return CompilationUnits.Peek().Builder.CreateCall(f, args, "")
}

func (v *AstVisitor) VisitFunctionReturn(ctx *parser.FunctionReturnContext) any {
	if ctx.Expression() != nil {
		return CompilationUnits.Peek().Builder.CreateRet(v.Visit(ctx.Expression()).(llvm.Value))
	} else {
		return CompilationUnits.Peek().Builder.CreateRetVoid()
	}
}

// -----------------------------
// Compiler's built-in functions
// -----------------------------

type functionType func([]any) llvm.Value

var functions map[string]functionType = map[string]functionType{
	"Cast":       Cast,
	"Sizeof":     SizeOf,
	"TypeNameOf": TypeNameOf,
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
		log.Fatal("ERROR: unknown value in @SizeOf function")
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

	return functions[name](arguments)
}
