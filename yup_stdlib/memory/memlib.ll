; yup memlib's standard functions

declare noalias i8* @malloc(i64 noundef)

declare noalias void @free(i8* noundef)

declare noalias i8* @memset(i8*, i32, i64)
