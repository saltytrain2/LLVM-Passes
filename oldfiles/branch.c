#include <stdio.h>

int* global_ptr = NULL;
int global_int = 5;
int* global_ptr2 = &global_int;

int main(){
    int x = 1;
    int y = 2;
    if (y != 2) {
        printf("wrong answer 1\n");
    }
    if (x != 1){
        printf("wrong answer 2\n");
    }

    if (global_ptr) {
        printf("correct answer");
    }
    else {
        printf("wrong answer 3");
    }
    
    if(global_ptr != NULL)
        printf("correct answer");
    else
        printf("wrong answer 3");
    return 0;
}