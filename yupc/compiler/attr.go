package compiler

import (
    	"strconv"
	"github.com/kamkow1/yup/yupc/parser"
)

const (
	AttrFuncGC int = 0
)

type Attribute struct {
	Name   string
	Params []string
}

func (v *AstVisitor) VisitAttributeList(ctx *parser.AttributeListContext) any {
	var attrs []*Attribute

	for _, a := range ctx.AllAttribute() {
		attrs = append(attrs, v.Visit(a).(*Attribute))
	}

	return attrs
}

func (v *AstVisitor) VisitAttribute(ctx *parser.AttributeContext) any {
	name := ctx.Identifier().GetText()
	var params []string
	for _, p := range ctx.AllValueString() {
    		p2, _ := strconv.Unquote(p.GetText())
		params = append(params, p2)
	}

	return &Attribute{name, params}
}
