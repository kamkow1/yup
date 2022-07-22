#include "stdio.h"

typedef int bool;
#define TRUE 1
#define FALSE 0

extern char get_char(char);
extern bool get_bool(bool);
extern int get_int(int);
extern float get_float(float);

int main(int argc, char *argv[])
{
    char ci;
    int ii;
    bool bi;
    float fi;

    // inputs
    printf("enter a character:\n");
    scanf("%c", &ci);

    printf("enter an integer:\n");
    scanf("%i", &ii);

    printf("enter a boolean (1 or 0):\n");
    scanf("%i", &bi);

    printf("enter a float:\n");
    scanf("%f", &fi);

    char c = get_char(ci);
    int i = get_int(ii);
    bool b = get_bool(bi);
    float f = get_float(fi);

    printf("char (i8) from yup %c\n", c);
    printf("int (i32) from yup %i\n", i);
    printf("bool (i8) from yup %i\n", b);
    printf("float from yup %f\n", f);

    return 0;
}
