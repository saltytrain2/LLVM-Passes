#include <stdio.h>

int main(){
    double i = 0.0000000000000001;
    if(i + 1 > 1)
        printf("correct answer");
    else
        printf("wrong answer");
    return 0;
}