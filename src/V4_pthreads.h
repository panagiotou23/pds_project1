#include <time.h>

#include <pthread.h> 

//Create a struct in order to pass multiple arguments
struct csc{
    int start,  //the start of the first loop
        stop,   //the end of the first loop
        * row,  //the CSC row vector
        * col;  //the CSC column vector
    float * c;  //the node vector that stores the triangles
};

//Pthread's function
void* find(void* arg){

    //Storing the argument in a variable
    struct csc * s = (struct csc * ) arg;

    //Using the same loop as V4 with different columns for each thread
    for(int i=s->start; i<s->start+s->stop; i++){
        for(int j=s->col[i]; j<s->col[i+1]; j++){
            int k=s->col[i];
            int l=s->col[s->row[j]];
            while(k<s->col[i+1] && l< s->col[s->row[j] +1]){
                if(s->row[l] == s->row[k]){
                    s->c[i] += 0.5;
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

long v4_pthreads(   int  * row, int * col,
                    float * c, int M, int nz, 
                    int num_threads)
{
    //The Variables used to time the function
    struct timespec ts_start;
    struct timespec ts_end;
    

    //Initialization of c
    for(int i=0; i<M; i++) c[i] = 0;

    //Creating the thread array
    pthread_t threads[num_threads];
    
    //Creating the struct array
    struct csc s[num_threads];

    //Start the clock
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    //Initializing the struct array
    if(M%num_threads == 0){                     //If the number of Columns is multiple to the number of threads 
        for(int i=0; i<num_threads; i++){
            s[i].c = c;
            s[i].row = row;
            s[i].col = col;
            s[i].stop = (M/num_threads);
            s[i].start = (M/num_threads)*i;
            
        }    
    }else{                                      //If not the residual will fall to the last thread
        for(int i=0; i<num_threads-1; i++){
            s[i].c = c;
            s[i].row = row;
            s[i].col = col;
            s[i].stop = (M/num_threads);
            s[i].start = (M/num_threads)*i;
            
        }
        s[num_threads - 1].c = c;
        s[num_threads - 1].row = row;
        s[num_threads - 1].col = col;
        s[num_threads - 1].start = (M/num_threads)*(num_threads - 1);
        s[num_threads - 1].stop = (M/num_threads) + M%num_threads;        
    }

    //Starting the threads
    for(int i=0; i<num_threads; i++) pthread_create(&threads[i], NULL, find, &s[i]);

    //And waiting for the threads to finish
    for(int i=0; i<num_threads; i++) pthread_join(threads[i], NULL);

    //Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    //Return the execution run-time
    return (ts_end.tv_sec - ts_start.tv_sec)* 1000000 + (ts_end.tv_nsec - ts_start.tv_nsec)/ 1000;

}
