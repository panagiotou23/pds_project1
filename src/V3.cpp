#include <iostream>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include "mmio.c"

using namespace std;

//For V1 & V2
int A[20][20], //the adjacency matrix initially 0
    c[20],
    wc[20];

//For V3
int *I, *J;
double *val;

void displayMatrix(int v) {
   int i, j;
   for(i = 0; i < v; i++) {
      for(j = 0; j < v; j++) {
         cout << A[i][j] << " ";
      }
      cout << endl;
   }
}

void add_edge(int u, int v) {       //function to add edge into the matrix
   A[u][v] = 1;
   A[v][u] = 1;
}

long find_triangle(){
    
    struct timespec ts_start;
    struct timespec ts_end;

    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    for(int i=0; i<6; i++){
        for(int j=0; j<i; j++){
            for(int k=0; k<j; k++){
                if(A[i][j] == 1 && A[j][k] == 1 && A[i][k] == 1){
                    c[i]++;
                    c[j]++;
                    c[k]++;
                    //cout << i << " " << j << " " << k << " \n";
                }  
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    return (ts_end.tv_nsec - ts_start.tv_nsec);
    //cout << "\n";
}

long find_wrong_triangle(){

    struct timespec ts_start;
    struct timespec ts_end;

    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    
    for(int i=0; i<6; i++){
        for(int j=0; j<6; j++){
            for(int k=0; k<6; k++){
                if(A[i][j] == 1 && A[j][k] == 1 && A[i][k] == 1){
                    wc[i]++;
                    wc[j]++;
                    wc[k]++;
                    //cout << i << " " << j << " " << k << " \n";
                }  
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    return (ts_end.tv_nsec - ts_start.tv_nsec);
    //cout << "\n";
}

int read_mat(int argc, char* argv[]){

    int ret_code;
    MM_typecode matcode;
    FILE *f;
    int M, N, nz;   
    int i;

    if (argc < 2)
	{
		fprintf(stderr, "Usage: %s [martix-market-filename]\n", argv[0]);
		exit(1);
	}
    else    
    { 
        if ((f = fopen(argv[1], "r")) == NULL) 
            exit(1);
    }

    if (mm_read_banner(f, &matcode) != 0)
    {
        printf("Could not process Matrix Market banner.\n");
        exit(1);
    }


    /*  This is how one can screen matrix types if their application */
    /*  only supports a subset of the Matrix Market data types.      */

    if (mm_is_complex(matcode) && mm_is_matrix(matcode) && 
            mm_is_sparse(matcode) )
    {
        printf("Sorry, this application does not support ");
        printf("Market Market type: [%s]\n", mm_typecode_to_str(matcode));
        exit(1);
    }

    /* find out size of sparse matrix .... */

    if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) !=0)
        exit(1);


    /* reseve memory for matrices */

    I = (int *) malloc(nz * sizeof(int));
    J = (int *) malloc(nz * sizeof(int));
    val = (double *) malloc(nz * sizeof(double));


    /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
    /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
    /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */

    for (i=0; i<nz; i++)
    {
        fscanf(f, "%d %d %lg\n", &I[i], &J[i], &val[i]);
        I[i]--;  /* adjust from 1-based to 0-based */
        J[i]--;
    }

    if (f !=stdin) fclose(f);

    /************************/
    /* now write out matrix */
    /************************/

    mm_write_banner(stdout, matcode);
    mm_write_mtx_crd_size(stdout, M, N, nz);
    for (i=0; i<nz; i++)
        fprintf(stdout, "%d %d %20.19g\n", I[i]+1, J[i]+1, val[i]);

	return 0;
}

void read_csc(int argc, char* argv[]){
    
}

main(int argc, char* argv[]) {

    read_mat(argc, argv);

}