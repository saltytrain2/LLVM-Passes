//addInvariant1.c - Prints the addresses of variables. In the future, the printAddress helper function will be mutated into the code instead of hard coded into the source.
#include <stdio.h>
#include <stdlib.h>

int x;

void printAddress(void* addr){
    printf("%p", &x);
}

int main() {
    x = 3;
    x = x + 4;
    printAddress(&x);
    return 0;
}
    
    
    