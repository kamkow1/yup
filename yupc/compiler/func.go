package compiler

import (
	"io/ioutil"
	"os/exec"
	"path"
	"path/filepath"
	"strings"
	"unsafe"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

// ----------------------
// User-defined functions
// ----------------------

type FuncParam struct {
	Name     string
	IsVarArg bool
	IsConst  bool
	Type     *TypeInfo
}

type Function struct {
	Name      string
	Value     *llvm.Value
	Params    []FuncParam
	ExitBlock *llvm.BasicBlock
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
			})
		} else {
			params = append(params, FuncParam{
				IsConst:  pp.KeywordConst() != nil,
				IsVarArg: false,
				Name:     pp.Identifier().GetText(),
				Type:     v.Visit(p).(*TypeInfo),
			})
		}
	}

	return params
}

func (v *AstVisitor) VisitFuncSig(ctx *parser.FuncSigContext) any {
	name := ctx.Identifier().GetText()
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
	isVarArg := false
	for _, fp := range params {
		if fp.IsVarArg {
			isVarArg = fp.IsVarArg
			continue
		}

		types = append(types, fp.Type.Type)
	}

	funcType := llvm.FunctionType(returnType.Type, types, isVarArg)
	function := llvm.AddFunction(CompilationUnits.Peek().Module, name, funcType)

	for i, pt := range params {
		if !pt.IsVarArg {
			function.Param(i).SetName(pt.Name)
		}
	}

	if ctx.KeywordPublic() == nil {
		function.SetLinkage(llvm.PrivateLinkage)
	} else {
		function.SetLinkage(llvm.LinkOnceAnyLinkage)
	}

	CompilationUnits.Peek().Functions[name] = Function{
		Name:      name,
		Value:     &function,
		Params:    params,
		ExitBlock: &llvm.BasicBlock{},
	}

	return function
}

func (v *AstVisitor) VisitFuncDef(ctx *parser.FuncDefContext) any {
	signature := v.Visit(ctx.FuncSig()).(llvm.Value)
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
		loc := LocalVariable{
			Name:    name,
			IsConst: true,
			Value:   a,
			IsUsed:  false,
		}
		CompilationUnits.Peek().Locals[len(CompilationUnits.Peek().Locals)-1][name] = loc
	}

	if signature.ParamsCount() > 0 {
		TrackedAllocsStack.Push(NewStack[llvm.Value]())

		for i, p := range signature.Params() {
			alloca := CreateAllocation(p.Type())
			loclen := len(CompilationUnits.Peek().Locals) - 1
			CompilationUnits.Peek().Locals[loclen][p.Name()] = LocalVariable{
				Name:    p.Name(),
				IsConst: function.Params[i].IsConst,
				Value:   alloca,
				IsUsed:  false,
			}
			CompilationUnits.Peek().Builder.CreateStore(p, alloca)
		}
	}

	exitStatus := v.Visit(ctx.CodeBlock()).(BlockExitStatus)
	hasTerminated := exitStatus.HasBranched || exitStatus.HasBrokenOut || exitStatus.HasContinued || exitStatus.HasReturned

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

	//return llvm.VerifyFunction(*function.Value, llvm.PrintMessageAction)
	return nil
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
		return Cast(args[0].(llvm.Value), args[1].(llvm.Type))
	},
	"size_of": func(args []any) llvm.Value {
		var result llvm.Value
		switch t := args[0].(type) {
		case llvm.Type:
			result = llvm.SizeOf(t)
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
		return llvm.ConstInt(llvm.Int1Type(), BoolToInt(err != nil), false)
	},
}

type BuiltInTypeFunction func([]any) llvm.Type

var BuiltInTypeFunctions map[string]BuiltInTypeFunction = map[string]BuiltInTypeFunction{
	"type_of": func(args []any) llvm.Type {
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
		f := CompilationUnits.Peek().Module.NamedFunction(name)
		if f.IsNil() {
			glb := CompilationUnits.Peek().Module.NamedGlobal(name)
			if !glb.IsNil() {
				f = CompilationUnits.Peek().Builder.CreateLoad(glb.Type().ElementType(), glb, "")
			} else {
				loc := FindLocalVariable(name, len(CompilationUnits.Peek().Locals)-1).Value
				f = CompilationUnits.Peek().Builder.CreateLoad(loc.AllocatedType(), loc, "")
			}
		}

		if f.IsNil() {
			LogError("tried to call an unknown function: %s", name)
		}

		var valueArgs []llvm.Value
		for _, a := range args {
			valueArgs = append(valueArgs, a.(llvm.Value))
		}

		return CompilationUnits.Peek().Builder.CreateCall(f.Type().ReturnType(), f, valueArgs, "")
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
			value = Cast(value, fncReturnType)
		}

		returnValue := FindLocalVariable("__return_value", len(CompilationUnits.Peek().Locals)-1)
		CompilationUnits.Peek().Builder.CreateStore(value, returnValue.Value)
		return CompilationUnits.Peek().Builder.CreateBr(*function.ExitBlock)
	} else if len(ctx.AllExpression()) > 1 {
		var vals []llvm.Value
		for _, expr := range ctx.AllExpression() {
			val := v.Visit(expr).(llvm.Value)
			if val.Type().TypeKind() != fncReturnType.TypeKind() {
				val = Cast(val, fncReturnType)
			}

			vals = append(vals, val)
		}

		return CompilationUnits.Peek().Builder.CreateAggregateRet(vals)
	}

	return CompilationUnits.Peek().Builder.CreateBr(*function.ExitBlock)
}

func Cast(value llvm.Value, typ llvm.Type) llvm.Value {
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

func BoolToInt(a bool) uint64 {
	return *(*uint64)(unsafe.Pointer(&a)) & 1
}