/*main.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

long next_thread_number;
int max_squares;
int total_open_tours;
int total_closed_tours;


int simulate( int argc, char ** argv );

int main( int argc, char ** argv ){
    
    next_thread_number = 1; 
    max_squares = total_open_tours = total_closed_tours = 0;
    int rc = simulate( argc, argv );
    return rc;
}
