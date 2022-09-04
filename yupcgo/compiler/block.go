package compiler

func CreateBlock() {
	m := map[string]LocalVariable{}
	compilationUnits.Peek().locals.Push(&m)
}

func RemoveBlock() *map[string]LocalVariable {
	return compilationUnits.Peek().locals.Pop()
}
