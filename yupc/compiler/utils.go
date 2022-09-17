package compiler

func TrimLeftChar(s string) string {
	for i := range s {
		if i > 0 {
			return s[i:]
		}
	}
	return s[:0]
}

func TrimRightChar(s string) string {
	return s[:len(s)-1]
}
