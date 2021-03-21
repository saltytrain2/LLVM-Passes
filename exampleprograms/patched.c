#include <stdio.h>
//pass return 0, fail return non-zero

int main() {
    
    int* global_ptr = NULL;

    if(global_ptr != NULL){
        //Code that would dereference global_ptr
        return 1;
    }

    if(global_ptr != NULL){
        //Code that would dereference global_ptr
        return 2;
    }

    if(global_ptr != NULL){
        //Code that would dereference global_ptr
        return 3;
    }
    
    if(global_ptr != NULL){
        //Code that would dereference global_ptr
        return 4;
    }

    return 0;
}
    