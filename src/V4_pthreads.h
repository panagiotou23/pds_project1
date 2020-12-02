#include <time.h>

#include <pthread.h> 

//Create a struct in order to pass multiple arguments
struct csc{
    int M,
        * row,  //the CSC row vector
        * col;  //the CSC column vector
    float * c;  //the node vector that stores the triangles
};

//Create a mutex to avoid a data race
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

//Create a global int to keep track of the iteration 
int iteration = 0;

//Pthread's function
void* find(void* arg){

    //Storing the argument in a variable
    struct csc * s = (struct csc * ) arg;

    //Initializing the iteration index
    pthread_mutex_lock(&m);
    int i = iteration;
    pthread_mutex_unlock(&m);

    //Using the same loop as V4 with different columns for each thread
    while(i < s->M){
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
        pthread_mutex_lock(&m);
        iteration++;
        i = iteration;
        pthread_mutex_unlock(&m);    
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
    for(int i=0; i<num_threads; i++){
        s[i].c = c;
        s[i].row = row;
        s[i].col = col;
        s[i].M = M;
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
