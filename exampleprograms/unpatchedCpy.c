//unpatched.c with a char overflow
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Program only takes in a string
int main(int argc, char *argv[]) {
    char buffer[] = "str";
    char buffer2[] = "test";
    strcpy(buffer, buffer2);
    
    if (!strncmp(buffer, "test", 4)) {
        return 0;
    }
   
    

    return 1; 
}
/*
//unpatched.c with a char overflow
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Program only takes in a string
int main(int argc, char *argv[]) {
    char buffer[15];
    int test = 0;

    strcpy(buffer, argv[1]);
    
    if (strncmp(buffer, "example", 15)) {
        test = 0;
    }
    else {
        test = 1;
    }
   
    if (test) {
        return 1;
    }
    

    return 0; //Return 0 == pass
}
    
*/
    
    