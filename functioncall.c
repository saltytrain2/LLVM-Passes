#include <stdio.h>

int plusOne(int a){
    return a + 1;
}
int subtractElements(int a, int b){
    return a - b;
}
int plusTwo(int a){
    return a + 2;
}
int main(){

    int num1 = 1;
    int num2 = 5;
    int num3 = 15;
    printf("Before: Num1 was %d\n", num1);
    printf("Before: Num2 was %d\n", num2);
    printf("Before: Num3 was %d\n", num3);
    
    num1 = plusOne(num1); //call plusTwo inst 33
    num2 = subtractElements(num2, num3); //swap params num3 - num2 instead of num2-num3, inst 37

    printf("After: Num1 was %d\n", num1);
    printf("After: Num2 was %d\n", num2);
    printf("After: Num3 was %d\n", num3);
    return 0;
}