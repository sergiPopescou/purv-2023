#include <stdio.h>
int main(int argc, char* argv[] ){
	int x = 10;
	int* ptr = &x;
	ptr++;
	printf("%x   %d \n",  ptr,  *ptr);  
} 

