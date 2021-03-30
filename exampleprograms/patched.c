#include <stdio.h>
#include <stdlib.h>

//Program only takes in positive numbers between 0-9
int main(int argc, char *argv[]) {

    int input = atoi(argv[1]);
    int overflow = 127; //-128 to 128
    int overflowwrapper = 127;


    int test1 = overflow + input;
    if(overflow < 0){
        return -1; //We overflowed return -1 to indicate a failure
    }

    return 0; //Return 0 == pass
}
    
    