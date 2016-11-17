int CAS (int *mem	,	int	testval	,	int	newval)
{
				int	res	=	*mem;
				if (* mem	==	testval)	*mem	=	newval;
				return	res;
}

/* My code */
/*Mutual exclusion using CAS*/

shared int lock=1;

while CAS(&lock, 1, 0){
	/*critical section*/
}
lock = 1
/*continue*/

/* CAS can be used to achieve wait free mutual exclusion, it is
very similar to test and set*/
