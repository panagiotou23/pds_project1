#include <time.h>
#include <stdio.h>
#include "mmio.c"

#include "coo2csc.h"

#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>

int *I, *J;                     //to store the COO matrix
double *val;                    //    >>      >>
int M, N, nz;                   //the number of rows, columns and non-zeros of the Matrix

int triangles;                  //to store the number of triangles in the Adjacency Matrix

uint32_t *csc_row, *csc_col;    //to store the CSC matrix

//To read the .mtx file
int read_mat(int argc, char* argv[]) {
    
    int ret_code;
    MM_typecode matcode;
    FILE *f;
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

    /* Replace missing val column with 1s and change the fscanf to match patter matrices*/

    if (!mm_is_pattern(matcode))
    {
    for (i=0; i<nz; i++)
    {
        fscanf(f, "%d %d %lg\n", &I[i], &J[i], &val[i]);
        I[i]--;  /* adjust from 1-based to 0-based */
        J[i]--;
    }
    }
    else
    {
    for (i=0; i<nz; i++)
    {
        fscanf(f, "%d %d\n", &I[i], &J[i]);
        val[i]=1;
        I[i]--;  /* adjust from 1-based to 0-based */
        J[i]--;
    }
    }

    if (f !=stdin) fclose(f);

    /************************/
    /* now write out matrix */
    /************************/

    mm_write_banner(stdout, matcode);
    mm_write_mtx_crd_size(stdout, M, N, nz);
    //for (i=0; i<nz; i++)
        //fprintf(stdout, "%d %d %20.19g\n", I[i]+1, J[i]+1, val[i]);

	return 0;
}

//To find the triangles in the Adjacency Matrix, returns the execution run-time
long find_triangle(int *c) {
    
    //Initializing the number of triangles
    triangles=0;

    //The Variables used to time the function
    struct timespec ts_start;
    struct timespec ts_end;

    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    
    for(int i=0; i<M-1; i++)
        for(int j=csc_col[i]; j<csc_col[i+1]; j++)
            for(int k=csc_col[csc_row[j]]; k<csc_col[csc_row[j] + 1]; k++)
                for(int l=j+1; l<csc_col[i+1]; l++)
                    if(csc_row[k] == csc_row[l])
                        triangles++;

    //Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    //Return the execution run-time
    return (ts_end.tv_sec - ts_start.tv_sec)* 1000000 + (ts_end.tv_nsec - ts_start.tv_nsec)/ 1000;
}

//Find triangles with OpenCilk
long find_triangle_cilk(int *c) {
    
    //Initializing the number of triangles
    triangles=0;

    //The Variables used to time the function
    struct timespec ts_start;
    struct timespec ts_end;

    CILK_C_REDUCER_OPADD(r, double, 0);
    CILK_C_REGISTER_REDUCER(r);

    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    
    // cilk::reducer< cilk::op_add<int> > parallel_sum(0);
    cilk_for(int i=0; i<M-1; i++)
        for(int j=csc_col[i]; j<csc_col[i+1]; j++)
            for(int k=csc_col[csc_row[j]]; k<csc_col[csc_row[j] + 1]; k++)           
                for(int l=j+1; l<csc_col[i+1]; l++)
                    if(csc_row[k] == csc_row[l])
                        REDUCER_VIEW(r)++;
                     
    CILK_C_UNREGISTER_REDUCER(r);
    triangles = REDUCER_VIEW(r);
    //Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    //Return the execution run-time
    return (ts_end.tv_sec - ts_start.tv_sec)* 1000000 + (ts_end.tv_nsec - ts_start.tv_nsec)/ 1000;
}

int main(int argc, char* argv[]) {

    //Read the given Matrix
    read_mat(argc, argv);

    //If not Square Matrix return 1
    if(M != N) exit(1);
    //If not Adjacency Matrix return 2
    for(int i=0; i<nz; i++) if(I[i] == J[i]) exit(2);
        
    //The Vetrices of Compressed Sparse Column
    csc_row = (uint32_t *)malloc(nz     * sizeof(uint32_t));
    csc_col = (uint32_t *)malloc((M + 1) * sizeof(uint32_t));

    //The Conversion from COO to CSC
    coo2csc(csc_row, csc_col,
            (uint32_t *)I, (uint32_t *)J,
            nz, M,
            0);

    //Cleanup the COO variables
    free(I);
    free(J);
    free(val);
    
    //The Vertex of nodes that stores the triangles
    int *c = (int *) malloc(M * sizeof(int));

    //Initialization of c
    for(int i=0; i<M; i++)
        c[i] = 0;

    // //Finding the triangles
    long seq_time = find_triangle(c);
    printf("V3: %ld us\n", seq_time);
    printf("%d\n\n", triangles);

    long cilk_time = find_triangle_cilk(c);
    printf("V3 Cilk: %ld us\n", cilk_time);
    printf("%d\n\n", triangles);

    printf("%lf%\n",100*(double)(seq_time-cilk_time)/seq_time);


    //Cleanup the CSC variables
    free(csc_row);
    free(csc_col);
    return 0;    
}
