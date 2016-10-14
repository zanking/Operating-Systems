#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

double t[0x02000000];
//2gb of ram
//~70 gb of ram
void segments()
{
    static int s = 42;
    double y = 100.69;
    void *p = malloc(1024);
    void *swag = malloc(2048);
    printf("stack\t%p\nstack2\t%p\nbrk\t%p\nheap\t%p\nheap\t%p"
           "static\t%p\nstatic\t%p\ntext\t%p\n",
           &p, &swag, sbrk(0), p, t, &s, &y, segments);
}

int main(int argc, char *argv[])
{
    segments();
    exit(0);
}
