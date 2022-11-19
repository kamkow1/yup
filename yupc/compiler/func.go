package compiler

import (
	"io/ioutil"
	"os/exec"
	"path"
	"path/filepath"
	"runtime"
	"strings"
	"unsafe"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

type FuncParam struct {
	Name     string
	IsVarArg bool
	IsConst  bool
	IsSelf   bool
	Type     *TypeInfo
}

type Function struct {
	Name           string
	Value          *llvm.Value
	Params         []FuncParam
	ExitBlock      *llvm.BasicBlock
	MethodName     string
	IsPublicMethod bool
	HasSelf        bool
}

func (v *AstVisitor) VisitFuncParamList(ctx *parser.FuncParamListContext) any {
	var params []FuncParam
	for _, p := range ctx.AllFuncParam() {
		pp := p.(*parser.FuncParamContext)
		if pp.SymbolVariadicArgs() != nil {
			params = append(params, FuncParam{
				IsVarArg: true,
				Name:     "vargs",
				Type:     nil,
				IsSelf:   false,
			})
		} else if pp.KeywordSelf() != nil {
			if len(StructNameStack.Units) <= 0 {
				LogError("cannot use self in a non-method function")
			}

			structName := StructNameStack.Peek()
			structType := CompilationUnits.Peek().Types[*structName]

			params = append(params, FuncParam{
				IsVarArg: false,
				IsConst:  pp.KeywordConst() != nil,
				Name:     "self",
				IsSelf:   true,
				Type: &TypeInfo{
					Type: llvm.PointerType(structType.Type, 0),
				},
			})
		} else {
			params = append(params, FuncParam{
				IsConst:  pp.KeywordConst() != nil,
				IsVarArg: false,
				IsSelf:   false,
				Name:     pp.Identifier().GetText(),
				Type:     v.Visit(p).(*TypeInfo),
			})
		}
	}

	return params
}

func (v *AstVisitor) VisitFuncSig(ctx *parser.FuncSigContext) any {
	name := ctx.Identifier().GetText()
	_, ok := CompilationUnits.Peek().Functions[name]
	if ok {
		LogError("function `%s` already exists in module `%s`",
			name, CompilationUnits.Peek().SourceFile)
	}

	var params []FuncParam
	if ctx.FuncParamList() != nil {
		params = v.Visit(ctx.FuncParamList()).([]FuncParam)
	} else {
		params = make([]FuncParam, 0)
	}

	var returnType *TypeInfo
	if ctx.TypeName() != nil {
		returnType = v.Visit(ctx.TypeName()).(*TypeInfo)
	} else {
		returnType = &TypeInfo{
			Type: llvm.VoidType(),
		}
	}

	var types []llvm.Type
	hasSelf := false
	isVarArg := false
	for _, fp := range params {
		if fp.IsVarArg {
			isVarArg = fp.IsVarArg
			continue
		}

		if fp.IsSelf {
			hasSelf = fp.IsSelf
		}

		types = append(types, fp.Type.Type)
	}

	funcType := llvm.FunctionType(returnType.Type, types, isVarArg)
	function := llvm.AddFunction(*CompilationUnits.Peek().Module, name, funcType)

	for i, pt := range params {
		if !pt.IsVarArg {
			function.Param(i).SetName(pt.Name)
		}
	}

	isPublic := ctx.KeywordPublic() != nil
	if isPublic {
		function.SetLinkage(llvm.LinkOnceAnyLinkage)
	} else if ctx.KeywordExtern() != nil {
		function.SetLinkage(llvm.ExternalLinkage)
	} else {
		function.SetLinkage(llvm.PrivateLinkage)
	}

	functionInfo := &Function{
		Name:           name,
		Value:          &function,
		Params:         params,
		ExitBlock:      &llvm.BasicBlock{},
		MethodName:     name,
		IsPublicMethod: isPublic,
		HasSelf:        hasSelf,
	}

	runtime.KeepAlive(functionInfo)

	CompilationUnits.Peek().Functions[name] = functionInfo
	// runtime.KeepAlive(functionInfo)
	return functionInfo
}

func (v *AstVisitor) VisitFuncDef(ctx *parser.FuncDefContext) any {
	signature := v.Visit(ctx.FuncSig()).(*Function).Value
	isVoid := signature.Type().ReturnType().ElementType().TypeKind() == llvm.VoidTypeKind
	function := CompilationUnits.Peek().Functions[signature.Name()]

	CreateBlock()
	bodyBlock := llvm.AddBasicBlock(*signature, "body")
	*function.ExitBlock = llvm.AddBasicBlock(*signature, "exit")

	CompilationUnits.Peek().Builder.SetInsertPointAtEnd(bodyBlock)

	if !isVoid {
		name := "__return_value"
		returnType := function.Value.Type().ReturnType().ElementType()
		a := CompilationUnits.Peek().Builder.CreateAlloca(returnType, name)
		loc := &LocalVariable{
			Name:    name,
			IsConst: true,
			Value:   a,
		}
		CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][name] = loc
	}

	if signature.ParamsCount() > 0 {
		TrackedAllocsStack.Push(NewStack[llvm.Value]())

		for i, p := range signature.Params() {
			alloca := CreateAllocation(p.Type())
			loclen := len(CompilationUnits.Peek().Locals) - 1
			CompilationUnits.Peek().Locals[loclen][p.Name()] = &LocalVariable{
				Name:    p.Name(),
				IsConst: function.Params[i].IsConst,
				Value:   alloca,
			}
			CompilationUnits.Peek().Builder.CreateStore(p, alloca)
		}
	}

	exitStatus := v.Visit(ctx.CodeBlock()).(BlockExitStatus)
	hasTerminated := exitStatus.HasBrokenOut || exitStatus.HasContinued || exitStatus.HasReturned

	if isVoid {
		if !hasTerminated || exitStatus.HasBranched {
			CompilationUnits.Peek().Builder.CreateBr(*function.ExitBlock)
		}

		CompilationUnits.Peek().Builder.SetInsertPointAtEnd(*function.ExitBlock)
		CompilationUnits.Peek().Builder.CreateRetVoid()
	} else {
		CompilationUnits.Peek().Builder.SetInsertPointAtEnd(*function.ExitBlock)
		returnValue := FindLocalVariable("__return_value", len(CompilationUnits.Peek().Locals)-1)
		load := CompilationUnits.Peek().Builder.CreateLoad(returnValue.Value.AllocatedType(), returnValue.Value, "")
		CompilationUnits.Peek().Builder.CreateRet(load)
	}

	if err := llvm.VerifyFunction(*function.Value, llvm.PrintMessageAction); err != nil {
		LogError("failed to verify function. read error message above")
	}

	return function
}

func (v *AstVisitor) VisitFuncCallArgList(ctx *parser.FuncCallArgListContext) any {
	var args []any
	for _, expr := range ctx.AllExpression() {
		args = append(args, v.Visit(expr))
	}

	return args
}

type BuiltInValueFunction func([]any) llvm.Value

var BuiltInValueFunctions map[string]BuiltInValueFunction = map[string]BuiltInValueFunction{
	"cast": func(args []any) llvm.Value {
		return Cast(args[0].(llvm.Value), args[1].(*TypeInfo))
	},
	"sizeof": func(args []any) llvm.Value {
		var result llvm.Value
		switch t := args[0].(type) {
		case *TypeInfo:
			result = llvm.SizeOf(t.Type)
		case llvm.Value:
			result = llvm.SizeOf(t.Type())
		}

		return result
	},
	"printable_type": func(args []any) llvm.Value {
		str := args[0].(llvm.Value).Type().String()
		return CompilationUnits.Peek().Builder.CreateGlobalStringPtr(str, "")
	},
	"range": func(args []any) llvm.Value {
		min := args[0].(int64)
		max := args[1].(int64)

		var vals []llvm.Value
		for i := min; i <= max; i++ {
			ci := llvm.ConstInt(llvm.Int64Type(), uint64(i), false)
			vals = append(vals, ci)
		}

		return llvm.ConstArray(llvm.Int64Type(), vals)
	},
	"inline_c": func(args []any) llvm.Value {
		cc := args[0].(string)

		sf := CompilationUnits.Peek().SourceFile
		name := filepath.Base(strings.TrimSuffix(sf, path.Ext(sf))) + "_c" + ".c"

		ioutil.WriteFile(name, []byte(cc), 0644)

		fpath := filepath.Join(GetCwd(), name)
		bcname := strings.TrimSuffix(name, path.Ext(name)) + ".bc"
		cmdargs := []string{"-c", "-emit-llvm", "-o", bcname, "-v", fpath}
		cmd := exec.Command("clang", cmdargs...)

		err := cmd.Run()
		return llvm.ConstInt(llvm.Int1Type(), boolToInt(err != nil), false)
	},
	"any_fmt": func(args []any) llvm.Value {
		value := args[0].(llvm.Value)
		terminator := args[1].(string)

		var formatString string

		if value.Type().TypeKind() == llvm.IntegerTypeKind {
			formatString = "%d" + terminator
		}

		if value.Type().String() == llvm.PointerType(llvm.Int8Type(), 0).String() {
			formatString = "%s" + terminator
		}

		if value.Type().TypeKind() == llvm.FloatTypeKind {
			formatString = "%f" + terminator
		}

		return CompilationUnits.Peek().Builder.CreateGlobalStringPtr(formatString, "fmt_str")
	},
}

type BuiltInTypeFunction func([]any) llvm.Type

var BuiltInTypeFunctions map[string]BuiltInTypeFunction = map[string]BuiltInTypeFunction{
	"typeof": func(args []any) llvm.Type {
		return args[0].(llvm.Value).Type()
	},
}

func (v *AstVisitor) VisitFuncCall(ctx *parser.FuncCallContext) any {
	name := ctx.Identifier().GetText()
	var args []any
	if ctx.FuncCallArgList() != nil {
		args = v.Visit(ctx.FuncCallArgList()).([]any)
	} else {
		args = make([]any, 0)
	}

	if f, ok := BuiltInValueFunctions[name]; ok {
		return f(args)
	} else if f1, ok1 := BuiltInTypeFunctions[name]; ok1 {
		return f1(args)
	} else {
		var funcToCall llvm.Value

		if _, ok := CompilationUnits.Peek().Functions[name]; ok {
			funcToCall = CompilationUnits.Peek().Module.NamedFunction(name)
		} else if global, ok2 := CompilationUnits.Peek().Globals[name]; ok2 {
			funcToCall = CompilationUnits.Peek().Builder.CreateLoad(global.Type().ElementType(), *global, "")
		} else {
			funcToCall = FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1).Value
			funcToCall = CompilationUnits.Peek().Builder.CreateLoad(funcToCall.Type().ElementType(), funcToCall, "")
		}

		var paramTypes []llvm.Type
		for _, arg := range args {
			switch arg.(type) {
			case llvm.Value:
				paramTypes = append(paramTypes, arg.(llvm.Value).Type())
			case llvm.Type:
				paramTypes = append(paramTypes, arg.(llvm.Type))
			}
		}

		// TODO: inform the user if they've passed a wrong number or arguments
		// currently this will result in a panic (not ideal)

		var valueArgs []llvm.Value
		for _, a := range args {
			valueArgs = append(valueArgs, a.(llvm.Value))
		}

		return CompilationUnits.Peek().Builder.CreateCall(funcToCall.Type().ReturnType(), funcToCall, valueArgs, "")
	}
}

func (v *AstVisitor) VisitFuncReturn(ctx *parser.FuncReturnContext) any {
	functionName := CompilationUnits.Peek().Builder.GetInsertBlock().Parent().Name()
	function := CompilationUnits.Peek().Functions[functionName]
	fncReturnType := CompilationUnits.Peek().Builder.GetInsertBlock().Parent().Type().ReturnType()
	fncReturnType = fncReturnType.ElementType()

	if ctx.AllExpression() != nil && len(ctx.AllExpression()) == 1 {
		value := v.Visit(ctx.Expression(0)).(llvm.Value)
		if value.Type() != fncReturnType {
			value = Cast(value, &TypeInfo{
				Type: fncReturnType,
			})
		}

		returnValue := FindLocalVariable("__return_value", len(CompilationUnits.Peek().Locals)-1)
		CompilationUnits.Peek().Builder.CreateStore(value, returnValue.Value)
		return CompilationUnits.Peek().Builder.CreateBr(*function.ExitBlock)
	} else if len(ctx.AllExpression()) > 1 {
		var vals []llvm.Value
		for _, expr := range ctx.AllExpression() {
			val := v.Visit(expr).(llvm.Value)
			if val.Type().TypeKind() != fncReturnType.TypeKind() {
				val = Cast(val, &TypeInfo{
					Type: fncReturnType,
				})
			}

			vals = append(vals, val)
		}

		return CompilationUnits.Peek().Builder.CreateAggregateRet(vals)
	}

	return CompilationUnits.Peek().Builder.CreateBr(*function.ExitBlock)
}

func boolToInt(a bool) uint64 {
	return *(*uint64)(unsafe.Pointer(&a)) & 1
}
