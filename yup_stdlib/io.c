#include <stdio.h>
#include <stdarg.h>

void Printf(char *format, ...) {
    va_list ap;
    va_start(ap, format);

    char *ptr = format;

    while(*ptr) {
        if (*ptr == '%') {
            ptr++;
            switch (*ptr++) {
                case 's':
                    printf("%s", va_arg(ap, char *));
                    break;
            }
        } else {
            putchar(*ptr++);
        }
    }

    va_end(ap);
}