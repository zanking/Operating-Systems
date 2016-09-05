#include <stdio.h>

int simpleadd(int number1, int number2, int *result){
	*result = number1+number2;
	return 0;
}

int main(){
	int *result=NULL;
	int number1 = 3;
	int number2 = 1;
	simpleadd(number1,number2, &result);
	printf("%d", result);
	return 0;
}
	
