#include <stdio.h>


void testFunc(int a, int b, int c) {
    printf("Arg1: %d\n", a); 
    printf("Arg2: %d\n", b);
    printf("Arg3: %d\n", c);
}


int main(){

    int num1 = 1;
    int num2 = 2;
    int num3 = 3;
    
    testFunc(num1, num2, num3);
    //TestFunc should print Arg1: 1 Arg2: 2 Arg3: (modified value)
    
    return 0;
}