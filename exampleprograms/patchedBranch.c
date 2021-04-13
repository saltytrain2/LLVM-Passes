//patchedBranch.c - If statements accurately reflect print statements.
#include <stdio.h>
#include <stdlib.h>

//Program takes in no input. We will rely output from printf to "evaluate" this file
int main() {
    int original = 0;

    if(original > 1){
        printf("Entered first if statement, condition was `original > 1`\n");
    }

    if(original != 0){
        printf("Entered second if statement, condition was `original != 1`\n");
    }

    if(!(original >= 1)){
        printf("Entered third if statement condition was `original >= 1`\n");
    }

    if(original == 0){
        printf("Entered fourth if statement condition was `original == 0`\n");
    }

    return 0;
}
    
    
    