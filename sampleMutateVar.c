#include <stdio.h>

double global_var = 0.0000000000000001;

int main(){
    if(global_var + 1 > 1)
        printf("correct answer");
    else
        printf("wrong answer");
    return 0;
}