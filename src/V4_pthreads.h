#include <time.h>

#include <pthread.h> 

//Create a struct in order to pass multiple arguments
struct csc{
    int i,      //the first loop index
        M,      //the size of the matrix
        * row,  //the CSC row vector
        * col;  //the CSC column vector
    float * c;  //the node vector that stores the triangles
};

//Using a mutex in order to avoid racing conditions
pthread_mutex_t m;

//Pthread's function
void* find(void* arg){

    //Storing the argument in a variable
    struct csc * s = (struct csc * ) arg;

    //A local variable to store the index of the first loop
    int i=0;
    
    while(1){
        //Storing the index 
        pthread_mutex_lock(&m);
        i = s->i++;
        pthread_mutex_unlock(&m);

        //Breaking the loop if it exceeds the size of the array
        if(i >= s->M) break;
        
        //Using the same loop as V4
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
    
    //Creating the struct
    struct csc s;
    s.c = c;
    s.row = row;
    s.col = col;
    s.i = 0;
    s.M = M;

    //Initializing the mutex
    pthread_mutex_init(&m, NULL);

    //Start the clock
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    //Starting the threads
    for(int i=0; i<num_threads; i++) pthread_create(&threads[i], NULL, find, &s);

    //And waiting for the threads to finish
    for(int i=0; i<num_threads; i++) pthread_join(threads[i], NULL);

    //Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    //Return the execution run-time
    return (ts_end.tv_sec - ts_start.tv_sec)* 1000000 + (ts_end.tv_nsec - ts_start.tv_nsec)/ 1000;

}
