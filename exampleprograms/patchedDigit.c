//patched.c without the char overflow
#include <stdio.h>
#include <stdlib.h>

//Testcase Digit tests numbers between 0-9
int main(int argc, char *argv[]) {

    int input = atoi(argv[1]);
    int overflow = 127; //Patched from char to int

    int test1 = overflow + input;
    if(test1 < 0){
        return -1; //We overflowed return -1 to indicate a failure
    }

    return 0; //Return 0 == pass
}
    
    