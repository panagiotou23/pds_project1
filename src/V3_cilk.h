#include <time.h>

#include <cilk/cilk.h>

long v3_cilk(    int * csc_row, int * csc_col,
            int * c, int M) 
{
    //The Variables used to time the function
    struct timespec ts_start;
    struct timespec ts_end;

    //Initialization of c
    for(int i=0; i<M; i++) c[i] = 0;

    //Start the clock
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    
    //Parallelizing the first for with Cilk
    cilk_for(int i=0; i<M; i++)
        for(int j=csc_col[i]; j<csc_col[i+1]; j++)
            for(int k=csc_col[csc_row[j]]; k<csc_col[csc_row[j] + 1]; k++)
                if(binarySearch(csc_row, j+1, csc_col[i+1] - 1, csc_row[k]) != -1) c[i]++;

                    
                        

    //Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    //Return the execution run-time
    return (ts_end.tv_sec - ts_start.tv_sec)* 1000000 + (ts_end.tv_nsec - ts_start.tv_nsec)/ 1000;
}

