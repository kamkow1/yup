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

var InlineAsm = "" // buffer for inline assembl

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

			return macro.Value
		}
	}

	return nil
}
