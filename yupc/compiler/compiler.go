package compiler

import (
	"fmt"
	"os"

	"github.com/antlr/antlr4/runtime/Go/antlr/v4"
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

type any = interface{}

func RunCompileJob(args []string) {
	llvm.InitializeAllTargetInfos()
	llvm.InitializeAllTargets()
	llvm.InitializeAllTargetMCs()
	llvm.InitializeAllAsmParsers()
	llvm.InitializeAllAsmPrinters()

	for _, fp := range args {
		ProcessPathRecursively(fp)
		WriteBCFile(GetBCWriteData())
		modname := CompilationUnits.Peek().ModuleName
		objpath := DumpObjectFile(modname)
		MakeExec(objpath, GlobalCompilerInfo.OutName)
	}	
}

type CompilerInfo struct {
	Line         int
	TargetTriple string
	Verbose      bool
	PrintModule  bool
	StaticLibs   []string
	OutName 	 string
}

func NewCompilerInfo() *CompilerInfo {
	return &CompilerInfo{
		Line:         0,
		TargetTriple: llvm.DefaultTargetTriple(),
		Verbose:      false,
		PrintModule:  false,
	}
}

type Stack[T any] struct {
	Units []*T
}

func NewStack[T any]() *Stack[T] {
	return &Stack[T]{}
}

func (s *Stack[T]) Push(item *T) {
	s.Units = append(s.Units, item)
}

func (s *Stack[T]) Pop() *T {
	cu := s.Units[len(s.Units)-1]
	s.Units = s.Units[0 : len(s.Units)-1]
	return cu
}

func (s *Stack[T]) Peek() *T {
	return s.Units[len(s.Units)-1]
}

type CompilationUnit struct {
	SourceFile string
	ModuleName string
	Builder    llvm.Builder
	Module     *llvm.Module
	Locals     []map[string]*LocalVariable
	Globals    map[string]*llvm.Value
	Functions  map[string]*Function
	Structs    map[string]*Structure
	Types 	   map[string]*TypeInfo
	Macros	   map[string]*MacroInfo
}

func NewCompilationUnit(sf string, bc string) *CompilationUnit {
	m := llvm.NewModule(sf)
	return &CompilationUnit{
		SourceFile: sf,
		ModuleName: bc,
		Builder:    llvm.NewBuilder(),
		Module:		&m,
		Locals:     []map[string]*LocalVariable{},
		Globals:    map[string]*llvm.Value{},
		Functions:  map[string]*Function{},
		Structs:    map[string]*Structure{},
		Macros:		map[string]*MacroInfo{},
		Types:		InitTypeMap(),
	}
}

var CompilationUnits = NewStack[CompilationUnit]()

func GetBCWriteData() (*llvm.Module, string) {
	mod := CompilationUnits.Peek().Module
	p := CompilationUnits.Peek().ModuleName
	return mod, p
}

var GlobalCompilerInfo *CompilerInfo = NewCompilerInfo()

type AstVisitor struct {
	*parser.BaseYupParserVisitor
}

func NewAstVisitor() *AstVisitor {
	return &AstVisitor{}
}

func (v *AstVisitor) Visit(tree antlr.ParseTree) any {
	switch ctx := tree.(type) {
	case *parser.FileContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFile(ctx)
	case *parser.CodeBlockContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitCodeBlock(ctx)
	case *parser.EmphasizedExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.Visit(ctx.Expression())
	case *parser.StatementContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStatement(ctx)
	case *parser.IncremDecremExprContext:
    		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
    		return v.VisitIncremDecremExpr(ctx);

	// Type
	case *parser.TypeAnnotContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitTypeName(ctx.TypeName().(*parser.TypeNameContext))
	case *parser.TypeNameContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitTypeName(ctx)
	case *parser.TypeExtContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitTypeExt(ctx)
	case *parser.ArrayTypeExtContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitArrayTypeExt(ctx)
	case *parser.LitTypeExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitTypeName(ctx.TypeName().(*parser.TypeNameContext))

	// Array
	//case *parser.ArrayElementAssignmentContext:
	//	GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
	//	return v.VisitArrayElementAssignment(ctx)
	case *parser.ArrayIndexContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitArrayIndex(ctx)
	case *parser.ConstArrayContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConstArray(ctx)
	case *parser.ConstArrayExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConstArray(ctx.ConstArray().(*parser.ConstArrayContext))
	case *parser.IndexedAccessExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitIndexedAccessExpr(ctx)

	// Variable
	case *parser.AssignContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitAssign(ctx)
	case *parser.VarDeclContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitVarDecl(ctx)
	case *parser.DeclTypeContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitDeclType(ctx)
	case *parser.VarValueContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitVarValue(ctx)
	case *parser.IdentifierExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitIdentifierExpr(ctx)

	// Function
	case *parser.FuncDefContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFuncDef(ctx)
	case *parser.FuncSigContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFuncSig(ctx)
	case *parser.FuncParamListContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFuncParamList(ctx)
	case *parser.FuncParamContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		annot := ctx.TypeAnnot().(*parser.TypeAnnotContext)
		name := annot.TypeName().(*parser.TypeNameContext)
		return v.VisitTypeName(name)
	case *parser.FuncReturnContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFuncReturn(ctx)
	case *parser.FuncCallContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFuncCall(ctx)
	case *parser.FuncCallArgListContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFuncCallArgList(ctx)
	case *parser.FuncCallExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFuncCall(ctx.FuncCall().(*parser.FuncCallContext))

	// Memory
	case *parser.AddressOfContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitAddressOf(ctx)
	case *parser.AddressOfExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitAddressOf(ctx.AddressOf().(*parser.AddressOfContext))
	case *parser.PtrDerefExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitPtrDerefExpr(ctx)
	// case *parser.DereferenceAssignmentContext:
	// GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
	// return v.VisitDereferenceAssignment(ctx)
	case *parser.ExprAssignContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitExprAssign(ctx)

	// Constant
	case *parser.ConstantContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConstant(ctx)
	case *parser.ConstantExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConstantExpression(ctx)
	case *parser.MultilineStringExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitMultilineStringExpression(ctx)
	case *parser.MultilineStringContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitMultilineString(ctx)
	case *parser.StringExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStringExpr(ctx)

	// Binary Operator
	case *parser.BinopContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitBinop(ctx)
	case *parser.BinopExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitBinopExpr(ctx)

	// Logic
	case *parser.CompOperContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitCompOper(ctx)
	case *parser.CompExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitCompExpr(ctx)
	case *parser.NegatedExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitNegatedExpression(ctx)
	case *parser.LogicalAndExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitLogicalAndExpression(ctx)
	case *parser.LogicalOrExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitLogicalOrExpression(ctx)

	// ifs
	case *parser.IfStatementContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitIfStatement(ctx)
	case *parser.IfThenBlockContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitIfThenBlock(ctx)
	case *parser.IfElseBlockContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitIfElseBlock(ctx)

	// Import
	case *parser.ImportDeclContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitImportDecl(ctx)

	// Loop
	case *parser.ForLoopStatementContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitForLoopStatement(ctx)
	case *parser.ConditionBasedLoopContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConditionBasedLoop(ctx)
	case *parser.StatementBasedLoopContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStatementBasedLoop(ctx)
	case *parser.ContinueStatementContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitContinueStatement(ctx)
	case *parser.BreakStatementContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitBreakStatement(ctx)

	// Struct
	case *parser.StructDeclarationContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStructDeclaration(ctx)
	case *parser.StructFieldContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStructField(ctx)
	case *parser.TypeAliasDeclarationContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitTypeAliasDeclaration(ctx)
	case *parser.FieldAccessExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitFieldAccessExpression(ctx)
	case *parser.StructInitExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStructInitExpression(ctx)
	case *parser.StructInitContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStructInit(ctx)
	case *parser.ConstStructInitExpressionContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConstStructInitExpression(ctx)
	case *parser.ConstStructInitContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitConstStructInit(ctx)
	case *parser.MethodCallExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitMethodCallExpr(ctx)
	case *parser.StaticMethodCallExprContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitStaticMethodCallExpr(ctx)

	case *parser.PreprocDeclContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.VisitPreprocDecl(ctx)
	case *parser.MacroRefContext:
		GlobalCompilerInfo.Line = ctx.GetStart().GetLine()
		return v.Visit(ctx.PreprocDecl())

	default:
		LogError("stepped into an unimplemented context: %s\n", ctx.GetText())
	}

	panic("ERROR: unreachable")
}

func (v *AstVisitor) VisitFile(ctx *parser.FileContext) any {
	for _, st := range ctx.AllStatement() {
		v.Visit(st)
	}

	return nil
}

func (v *AstVisitor) VisitStatement(ctx *parser.StatementContext) any {
	return v.Visit(ctx.GetChild(0).(antlr.ParseTree))
}

func LogError(f string, vars ...any) {
	base := fmt.Sprintf(
		"%s:%d",
		CompilationUnits.Peek().SourceFile,
		GlobalCompilerInfo.Line,
	)

	msg := fmt.Sprintf(f, vars...)

	fmt.Println(fmt.Sprintf("%s %s", base, msg))
	os.Exit(1)
}
