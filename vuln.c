#include <stdint.h>
#include <stdio.h>
#include <string.h> // for memcpy
#include <inttypes.h>
#define CAN_MSGAVAIL        (3)

int global_variable = 5;
int global_variable1 = 10;
unsigned char readMsgBufID(unsigned long *ID, unsigned char *len, unsigned char *buf);


unsigned long g_ID;
unsigned char g_len;
unsigned char g_buf[8];
unsigned char g_res;

unsigned char readMsgBufID(unsigned long *ID, unsigned char *len, unsigned char *buf)
{
	memcpy(ID, &g_ID, sizeof(g_ID));
	memcpy(len, &g_len, sizeof(g_len));
	memcpy(buf, &g_buf, sizeof(g_buf));
	(*len) = (*len) % 8;

	return g_res;
}

int foo(unsigned char buf[])
{
	unsigned char len = 0;
	unsigned long id = 0;
	readMsgBufID(&id, &len, buf);
    
	uint16_t speed_value = (buf[3] << 8) + buf[2];  // <-------- Vulnerability here
	//uint16_t speed_value = 10000000000;  // <-------- Vulnerability here

	if ( speed_value > 0 ) {
        printf("speed over 0\n");
		return 1;
	}

	global_variable ++;
	global_variable1 --;

	printf("hello from foo!\n");
	return 0;
}

int main ()
{
	unsigned char x[8] = {1};
	return foo(x);
}


