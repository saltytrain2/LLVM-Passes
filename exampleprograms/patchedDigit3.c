//unpatched.c with a char overflow
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
int main(int argc, char *argv[]) {

    int input = atoi(argv[1]);
    char overflow = 127; //-128 to 128

    if (CHAR_MAX - input < overflow) {
        return 0;
    }
    char test1 = overflow + input;
    if(test1 < 0){
        return -1; //We overflowed return -1 to indicate a failure
    }

    return 0; //Return 0 == pass
}
    
    