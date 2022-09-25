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
	Name     string
	IsVarArg bool
	Type     llvm.Type
}

type Function struct {
	Name      string
	Value     llvm.Value
	Params    []FuncParam
	ExitBlock *llvm.BasicBlock
}

func (v *AstVisitor) VisitFunctionParameter(ctx *parser.FunctionParameterContext) any {
	return v.Visit(ctx.TypeAnnotation())
}

func (v *AstVisitor) VisitFunctionParameterList(ctx *parser.FunctionParameterListContext) any {
	var params []FuncParam
	for _, p := range ctx.AllFunctionParameter() {
		if p.(*parser.FunctionParameterContext).SymbolVariadicArgs() != nil {
			params = append(params, FuncParam{
				IsVarArg: true,
				Name:     "vargs",
				Type:     llvm.Type{},
			})
		} else {
			params = append(params, FuncParam{
				IsVarArg: false,
				Name:     p.(*parser.FunctionParameterContext).Identifier().GetText(),
				Type:     v.Visit(p).(llvm.Type),
			})
		}
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
	isVarArg := false
	for _, fp := range params {
		if fp.IsVarArg {
			isVarArg = fp.IsVarArg
			continue
		}

		types = append(types, fp.Type)
	}

	funcType := llvm.FunctionType(returnType, types, isVarArg)
	function := llvm.AddFunction(CompilationUnits.Peek().Module, name, funcType)

	for i, pt := range params {
		if pt.IsVarArg {
			continue
		}

		function.Param(i).SetName(pt.Name)
	}

	if ctx.AttributeList() != nil {
		attrs := v.Visit(ctx.AttributeList()).([]*Attribute)
		for _, a := range attrs {
			switch a.Name {
			case "gc":
				{
					gc := a.Params[0]
					if gc == "default" {
						function.SetGC("coreclr")
					} else {
						function.SetGC(gc)
					}
				}
			case "link_type":
				{
					linkage := a.Params[0]
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
	*function.ExitBlock = llvm.AddBasicBlock(signature, "exit")

	CompilationUnits.Peek().Builder.SetInsertPointAtEnd(bodyBlock)

	if !isVoid {
		name := "__return_value"
		returnType := function.Value.Type().ReturnType().ElementType()
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

	hasTerminated := v.Visit(ctx.CodeBlock()).(bool)

	if isVoid {
		if !hasTerminated {
			CompilationUnits.Peek().Builder.CreateBr(*function.ExitBlock)
		}

		CompilationUnits.Peek().Builder.SetInsertPointAtEnd(*function.ExitBlock)
		return CompilationUnits.Peek().Builder.CreateRetVoid()
	} else {
		CompilationUnits.Peek().Builder.SetInsertPointAtEnd(*function.ExitBlock)
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

type BuiltInValueFunction func([]any) llvm.Value

var BuiltInValueFunctions map[string]BuiltInValueFunction = map[string]BuiltInValueFunction{
	"cast":          Cast,
	"size_of":       SizeOf,
	"is_type_equal": IsTypeEqual,
	"type_to_str":   TypeToStr,
	"range":         Range,
}

type BuiltInTypeFunction func([]any) llvm.Type

var builtInTypeFunctions map[string]BuiltInTypeFunction = map[string]BuiltInTypeFunction{
	"type_of": TypeOf,
}

func (v *AstVisitor) VisitFunctionCall(ctx *parser.FunctionCallContext) any {
	name := ctx.Identifier().GetText()
	var args []any
	if ctx.FunctionCallArgList() != nil {
		arglist := ctx.FunctionCallArgList().(*parser.FunctionCallArgListContext)
		for _, a := range arglist.AllExpression() {
			args = append(args, v.Visit(a))
		}
	} else {
		args = make([]any, 0)
	}

	if f, ok := BuiltInValueFunctions[name]; ok {
		return f(args)
	} else {
		f := CompilationUnits.Peek().Module.NamedFunction(name)
		if f.IsNil() {
			log.Fatalf("ERROR: tried to call an unknown function: %s", name)
		}

		var valueArgs []llvm.Value
		for _, a := range args {
			valueArgs = append(valueArgs, a.(llvm.Value))
		}

		return CompilationUnits.Peek().Builder.CreateCall(f, valueArgs, "")
	}
}

func (v *AstVisitor) VisitFunctionReturn(ctx *parser.FunctionReturnContext) any {
	functionName := CompilationUnits.Peek().Builder.GetInsertBlock().Parent().Name()
	function := CompilationUnits.Peek().Functions[functionName]
	if ctx.Expression() != nil {
		value := v.Visit(ctx.Expression()).(llvm.Value)
		returnValue := FindLocalVariable("__return_value", len(CompilationUnits.Peek().Locals)-1)
		CompilationUnits.Peek().Builder.CreateStore(value, returnValue.Value)
		return CompilationUnits.Peek().Builder.CreateBr(*function.ExitBlock)
	}

	return CompilationUnits.Peek().Builder.CreateBr(*function.ExitBlock)
}

// -----------------------------
// Compiler's built-in functions
// -----------------------------

func TypeOf(args []any) llvm.Type {
	expr := args[0].(llvm.Value)
	return expr.Type()
}

func TypeToStr(args []any) llvm.Value {
	expr := args[0].(llvm.Type)
	str := expr.String()
	return CompilationUnits.Peek().Builder.CreateGlobalStringPtr(str, "")
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

func Range(args []any) llvm.Value {

	min := args[0].(int64)
	max := args[1].(int64)

	var vals []llvm.Value
	for i := min; i <= max; i++ {
		ci := llvm.ConstInt(llvm.Int64Type(), uint64(i), false)
		vals = append(vals, ci)
	}

	return llvm.ConstArray(llvm.Int64Type(), vals)
}
