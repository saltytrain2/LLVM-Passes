#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    
    int* global_ptr = NULL;

    if(global_ptr != NULL){
        *global_ptr = 4;
    }

    if(global_ptr != NULL){
        *global_ptr = 5;
    }

    if(global_ptr != NULL){
        *global_ptr = 6;
    }
    
    if(global_ptr != NULL){
        *global_ptr = 7;
    }

    return 0;
}
    