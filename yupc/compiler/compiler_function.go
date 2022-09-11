package compiler

import (
	"fmt"

	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

type functionType func([]string) llvm.Value

var functions map[string]functionType = map[string]functionType{
	"Cast":   Cast,
	"Sizeof": Sizeof,
}

func Cast(args []string) llvm.Value {
	value := FindLocalVariable(args[0], len(CompilationUnits.Peek().Locals)-1).Value
	load := CompilationUnits.Peek().Builder.CreateLoad(value, "")
	typ, ok := BuiltinLLVMTypes[args[1]]
	if !ok {
		panic(fmt.Sprintf("ERROR: unknown type: %s", args[1]))
	}

	if load.Type().TypeKind() == llvm.IntegerTypeKind && typ.TypeKind() == llvm.IntegerTypeKind {
		return CompilationUnits.Peek().Builder.CreateIntCast(load, typ, "")
	} else if load.Type().TypeKind() == llvm.IntegerTypeKind && typ.TypeKind() == llvm.PointerTypeKind {
		return CompilationUnits.Peek().Builder.CreateIntToPtr(load, typ, "")
	} else if load.Type().TypeKind() == llvm.PointerTypeKind && typ.TypeKind() == llvm.IntegerTypeKind {
		return CompilationUnits.Peek().Builder.CreatePtrToInt(load, typ, "")
	} else if load.Type().TypeKind() == llvm.PointerTypeKind && typ.TypeKind() == llvm.PointerTypeKind {
		return CompilationUnits.Peek().Builder.CreatePointerCast(load, typ, "")
	} else {
		return CompilationUnits.Peek().Builder.CreateBitCast(load, typ, "")
	}
}

func Sizeof(args []string) llvm.Value {
	typ, ok := BuiltinLLVMTypes[args[0]]
	if ok {
		return llvm.SizeOf(typ)
	}

	value := FindLocalVariable(args[0], len(CompilationUnits.Peek().Locals)-1).Value
	load := CompilationUnits.Peek().Builder.CreateLoad(value, "")
	return llvm.SizeOf(load.Type())
}

func (v *AstVisitor) VisitYupFunctionExpression(ctx *parser.YupFunctionExpressionContext) any {
	return v.Visit(ctx.YupFunction())
}

func (v *AstVisitor) VisitYupFunction(ctx *parser.YupFunctionContext) any {
	callCtx := ctx.FunctionCall().(*parser.FunctionCallContext)
	name := callCtx.Identifier().GetText()
	var strArgs []string
	args := callCtx.FunctionCallArgList().(*parser.FunctionCallArgListContext)
	for _, a := range args.AllExpression() {
		strArgs = append(strArgs, a.GetText())
	}

	return functions[name](strArgs)
}
