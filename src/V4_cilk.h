#include <time.h>

#include <cilk/cilk.h>

long v4_cilk(   int  * row, int * col, int * val, 
                float * c, int M, int nz)
{

    //The Variables used to time the function
    struct timespec ts_start;
    struct timespec ts_end;

    for(int i=0; i<M; i++) c[i] = 0;

    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    cilk_for(int i=0; i<M; i++){
        for(int j=col[i]; j<col[i+1]; j++){
            int k=col[i];
            int l=col[row[j]];
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

    //Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    //Return the execution run-time
    return (ts_end.tv_sec - ts_start.tv_sec)* 1000000 + (ts_end.tv_nsec - ts_start.tv_nsec)/ 1000;

}
