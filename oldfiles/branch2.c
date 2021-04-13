#include <stdio.h>
#include <stdint.h>
int16_t globalint = 127;
int16_t globalint2 = 0;
 
int main(){
    globalint = globalint + 1;
    globalint2 = globalint;
    if (globalint == 128) {
        printf("Global int is 16 bits wide\n");
    }
    else if(globalint == -128){
        printf("Global int is not 16 bits wide\n");
    }
    if (globalint2 == 128) {
        printf("Global int2 is 16 bits wide\n");
    }
    else if(globalint2 == -128){
        printf("Global int2 is not 16 bits wide\n");
    }    
    return 0;
}