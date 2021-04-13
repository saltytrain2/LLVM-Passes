//patched.c without the char overflow
#include <stdio.h>
#include <stdlib.h>

//Program only takes in positive numbers between 0-9
int main() {
    int original = 0

    if(original > 1){
        return -1; //We overflowed return -1 to indicate a failure
    }

    if(original != 0){
        return -1;
    }

    if(!(original == 0)){
        return -1;
    }

    if(original )

    return 0; //Return 0 == pass
}
    
    
    