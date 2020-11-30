#include <time.h>

#include <pthread.h> 

struct csc{
    int start,
        stop,
        * row,
        * col,
        *val;
    float * c;
};

void* find(void* arg){

    struct csc * s = (struct csc * ) arg;

    float *sum = 0;

    for(int i=s->start; i<s->start+s->stop; i++){
        for(int j=s->col[i]; j<s->col[i+1]; j++){
            int k=s->col[i];
            int l=s->col[s->row[j]];
            while(k<s->col[i+1] && l< s->col[s->row[j] +1]){
                if(s->row[l] == s->row[k]){
                    s->c[i] += (float)s->val[j]/2;
                    k++;
                    l++; 
                }else if(s->row[l] > s->row[k]){
                    k++;
                }else{
                    l++;
                }
            }
        }
    }
    return NULL;
}

long v4_pthreads(int  * row, int * col, int * val, 
            float * c, int M, int nz, int num_threads)
{
    //The Variables used to time the function
    struct timespec ts_start;
    struct timespec ts_end;
    
    for(int i=0; i<M; i++) c[i] = 0;

    pthread_t threads[num_threads];
    
    struct csc s[num_threads];

    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    if(M%num_threads == 0){
        for(int i=0; i<num_threads; i++){
            s[i].c = c;
            s[i].row = row;
            s[i].col = col;
            s[i].val = val;
            s[i].stop = (M/num_threads);
            s[i].start = (M/num_threads)*i;
            
        }    
    }else{
        for(int i=0; i<num_threads-1; i++){
            s[i].c = c;
            s[i].row = row;
            s[i].col = col;
            s[i].val = val;
            s[i].stop = (M/num_threads);
            s[i].start = (M/num_threads)*i;
            
        }
        s[num_threads - 1].c = c;
        s[num_threads - 1].row = row;
        s[num_threads - 1].col = col;
        s[num_threads - 1].val = val;
        s[num_threads - 1].start = (M/num_threads)*(num_threads - 1);
        s[num_threads - 1].stop = (M/num_threads) + M%num_threads;        
    }

    for(int i=0; i<num_threads; i++){
        pthread_create(&threads[i], NULL, find, &s[i]);
    }

    for(int i=0; i<num_threads; i++) pthread_join(threads[i], NULL);

    //Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    //Return the execution run-time
    return (ts_end.tv_sec - ts_start.tv_sec)* 1000000 + (ts_end.tv_nsec - ts_start.tv_nsec)/ 1000;

}
