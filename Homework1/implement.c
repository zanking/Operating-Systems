#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

/*int simpleadd(int number1, int number2, int *result){
	*result = number1+number2;
	return 0;
}*/

int main(){

	int number1 = 3;
	int number2 = 1;
	int *result=NULL;
	int x = syscall(326,11,12,&result);//simpleadd(number1,number2, &result);
	printf("%d\n", result);
	printf("%d\n", x);
	return 0;
}
