// Nicholas Clement

//This is a simple fork program 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sched.h>


int main( void )
{
    printf( "[dad] pid %d\n", getpid() );

    for ( int i = 0; i < 3; i++ )
        if ( fork() == 0 )
        {
            printf( "[son] pid %d from pid %d\n", getpid(), getppid() );
            exit( 0 );
        }

    for ( int i = 0; i < 3; i++ )
        wait( NULL );
}
