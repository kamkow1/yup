package compiler

func CreateBlock() {
	m := map[string]LocalVariable{}
	compilationUnits.Peek().locals = append(compilationUnits.Peek().locals, m)
}

func RemoveBlock() []map[string]LocalVariable {
	return RemoveIndex(compilationUnits.Peek().locals, len(compilationUnits.Peek().locals)-1)
}
