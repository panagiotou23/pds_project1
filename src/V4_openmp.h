#include <time.h>

#include <omp.h>

long v4_openmp( int  * row, int * col, 
                float * c, int M, int nz)
{

    //The Variables used to time the function
    struct timespec ts_start;
    struct timespec ts_end;
    
    //Initialization of c
    for(int i=0; i<M; i++) c[i] = 0;

    //Declaring the private variables of the OpenMP Parallelization
    int i,j,k,l;

    //Start the clock
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    //Starting the OpenMP Parallelization
    #pragma omp parallel shared(row, col) private(i, j, k, l) 
    {
        //Parallelizing for
        #pragma omp for schedule(dynamic)
        for(i=0; i<M; i++){
            for(j=col[i]; j<col[i+1]; j++){
                k=col[i];
                l=col[row[j]];
                while(k<col[i+1] && l< col[row[j] +1]){
                    if(row[l] == row[k]){
                        c[i] += 0.5;
                        k++;
                        l++; 
                    }else if(row[l] > row[k]){
                        k++;
                    }else{
                        l++;
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
