#include <stdio.h>
#include <stdint.h>

void fun(int val1, double val2)
{
    double counter = 24;
    counter /= val2;

    double i = val1 + counter;
    printf("The original value of i is 11.286\n");
    printf("The value of i is %f\n", i);
    return;
}

int main() 
{
    int32_t i = 0xFFFF;
    int64_t k = (int64_t)i;
    double j = (double)k;
    
    fun(i, j);
    return 0;
}