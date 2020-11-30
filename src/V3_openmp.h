#include <time.h>
#include <stdio.h>

#include <omp.h>

//Find triangles with OpenMP
long v3_openmp(  int * csc_col, int * csc_row,
                            int * c, int M) 
{
    //The Variables used to time the function
    struct timespec ts_start;
    struct timespec ts_end;

    //Initialization of c
    for(int i=0; i<M; i++) c[i] = 0;

    int i,j,k,l;

    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    #pragma omp parallel shared(csc_row, csc_col) private(i, j, k, l)
    {
        #pragma omp for schedule(dynamic)
        for(i=0; i<M; i++){
            for(j=csc_col[i]; j<csc_col[i+1]; j++){
                for(k=csc_col[csc_row[j]]; k<csc_col[csc_row[j] + 1]; k++){ 
                    for(l=j+1; l<csc_col[i+1]; l++){
                        if(csc_row[k] == csc_row[l]){
                            c[i]++;
                        } 
                    }
                }
            }
        }
    }
    
    //Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    //Return the execution run-time
    return (ts_end.tv_sec - ts_start.tv_sec)* 1000000 + (ts_end.tv_nsec - ts_start.tv_nsec)/ 1000;
}
