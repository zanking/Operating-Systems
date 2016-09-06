#include	<linux/kernel.h>
#include	<linux/linkage.h>
asmlinkage	long	sys_simpleadd(int number1, int number2, int *result){
	
	*result = number1 + number2;
	printk(KERN_ALERT "%d\n%d\n%d\n", number1, number2, *result);
	return 0;
}

//int simpleadd(int number1, int number2, int *result)
//	*result = number1+number2;
//	return 0;
//}

