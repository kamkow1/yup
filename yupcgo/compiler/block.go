package compiler

func CreateBlock() {
	m := map[string]LocalVariable{}
	CompilationUnits.Peek().Locals = append(CompilationUnits.Peek().Locals, m)
}

func RemoveBlock() []map[string]LocalVariable {
	return RemoveIndex(CompilationUnits.Peek().Locals, len(CompilationUnits.Peek().Locals)-1)
}
