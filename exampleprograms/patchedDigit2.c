//unpatched.c with a char overflow
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    long int input = atoi(argv[1]);
    long int overflow = 127; //-128 to 128

    long int test1 = overflow + input;
    if(test1 < 0){
        return -1; //We overflowed return -1 to indicate a failure
    }

    return 0; //Return 0 == pass
}
    
    