//unpatched.c with a char overflow
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Program only takes in a string
int main(int argc, char *argv[]) {
    char buffer[] = "str";
    char buffer2[] = "test";
    strncpy(buffer, buffer2,3);
    
    if (!strncmp(buffer, "test", 4)) {
        return 0;
    }
   
    

    return 1; 
}

    