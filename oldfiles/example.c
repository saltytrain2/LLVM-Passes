#include <stdio.h>

int doStuff(int num) {
    num = num + 2;
    num = num + 2;
    return num;
}

int main(int argc, const char** argv) {
    int num;
    scanf("%i", &num);
    num = doStuff(num);
    printf("Was + %i\n", num + 2);
    printf("Was - %i\n", num - 2);
    printf("Was / %i\n", num / 2);
    printf("Was * %i\n", num * 2);
    printf("Was + %i\n", num + 2);

    return 0;
}
