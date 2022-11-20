package compiler

import (
	"github.com/antlr/antlr4/runtime/Go/antlr/v4"
	"github.com/kamkow1/yup/yupc/parser"
	"tinygo.org/x/go-llvm"
)

type MacroInfo struct {
	Name     string
	IsPublic bool
	Value    any
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
				//Value:    v.Visit(ctx.Expression()).(llvm.Value),
			}

			if ctx.Expression() != nil {
				macro.Value = ctx.Expression()
			}

			if ctx.CodeBlock() != nil {
				macro.Value = ctx.CodeBlock()
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

			var value llvm.Value

			if !ok {
				LogError("tried to reference an unknown macro `%s`", name)
			}

			switch name {
			case "current_func":
				{
					currentFunc := CompilationUnits.Peek().Builder.GetInsertBlock().Parent().Name()
					value = CompilationUnits.Peek().Builder.CreateGlobalStringPtr(currentFunc, "")
				}
			case "near_line":
				value = llvm.ConstInt(llvm.Int64Type(), uint64(GlobalCompilerInfo.Line), false)
			default:
				{
					switch macro.Value.(type) {
					case *parser.CodeBlockContext:
						v.VisitCodeBlock(macro.Value.(*parser.CodeBlockContext))
					default:
						value = v.Visit(macro.Value.(antlr.ParseTree)).(llvm.Value)
					}
				}
			}

			return value
		}
	}

	return nil
}
