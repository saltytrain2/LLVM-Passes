//unpatched.c with a char overflow
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Program only takes in a string
int main(int argc, char *argv[]) {
    char buffer[8] = "str";
    char buffer2[8] = "test";
    strcpy(buffer, buffer2);
    
    if (!strncmp(buffer, "test", 8)) {
        return 0;
    }
   
    

    return 1; 
}