#include <stdio.h>
#include <stdarg.h>

void fmt_print(char *format, ...) {
    va_list ap;
    va_start(ap, format);

    char *ptr = format;

    while(*ptr) {
        if (*ptr == '%') {
            ptr++;
            switch (*ptr++) {
                case 's':
                    printf("%s", va_arg(ap, char*));
                    break;
                case 'i':
                    printf("%d", va_arg(ap, int));
                    break;
                case 'f':
                    printf("%.15f", va_arg(ap, double));
                    break;
                case 'c':
                    printf("%c", va_arg(ap, int));
                    break;
            }
        } else {
            putchar(*ptr++);
        }
    }

    va_end(ap);
}
