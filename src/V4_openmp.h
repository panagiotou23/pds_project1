#include <time.h>

#include <omp.h>

long v4_openmp( int  * row, int * col, int * val, 
                float * c, int M, int nz)
{

    //The Variables used to time the function
    struct timespec ts_start;
    struct timespec ts_end;
    
    int i, j, k, l;

    for(int i=0; i<M; i++) c[i] = 0;

    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    #pragma omp parallel shared(row, col, val) private(i, j, k, l) 
    {
        #pragma omp for schedule(dynamic)
        for(i=0; i<M; i++){
            for(j=col[i]; j<col[i+1]; j++){
                k=col[i];
                l=col[row[j]];
                while(k<col[i+1] && l< col[row[j] +1]){
                    if(row[l] == row[k]){
                        c[i] += (float)val[j]/2;
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
