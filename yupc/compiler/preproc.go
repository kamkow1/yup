package compiler

import (
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

type MacroInfo struct {
	Name     string
	IsPublic bool
	Value    llvm.Value
	// TODO: add macros with blocks
	// TODO: add macro type
}

const (
	Macro = "macro"
	Asm   = "asm"
)

var InlineAsm = "" // buffer for inline assembly

func InitMacroMap() map[string]*MacroInfo {
	macros := map[string]*MacroInfo{
		"current_func": &MacroInfo{
			Name:     "current_func",
			IsPublic: true,
		},
		"near_line": &MacroInfo{
			Name:     "near_line",
			IsPublic: true,
		},
	}

	return macros
}

func (v *AstVisitor) VisitPreprocDecl(ctx *parser.PreprocDeclContext) any {
	// macro type name
	switch ctx.Identifier(0).GetText() {
	case Macro:
		{
			macroId := ctx.Identifier(1).GetText()
			macro := &MacroInfo{
				Name:     macroId,
				IsPublic: false, // TODO: visibility modifiers for macros
				Value:    v.Visit(ctx.Expression()).(llvm.Value),
			}

			CompilationUnits.Peek().Macros[macroId] = macro
		}
	case Asm:
		{
			asmText := v.Visit(ctx.Expression()).(string)
			InlineAsm += asmText
			//CompilationUnits.Peek().Module.SetInlineAsm(asmText)
		}
	default:
		{
			name := ctx.Identifier(0).GetText()
			macro, ok := CompilationUnits.Peek().Macros[name]

			if !ok {
				LogError("tried to reference an unknown macro `%s`", name)
			}

			// macros that depend on the current location / interpreter state
			if name == "current_func" {
				currentFunc := CompilationUnits.Peek().Builder.GetInsertBlock().Parent().Name()
				macro.Value = CompilationUnits.Peek().Builder.CreateGlobalStringPtr(currentFunc, "")
			}

			if name == "near_line" {
				macro.Value = llvm.ConstInt(llvm.Int64Type(), uint64(GlobalCompilerInfo.Line), false)
			}

			return macro.Value
		}
	}

	return nil
}
