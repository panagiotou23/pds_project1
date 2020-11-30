#include <stdlib.h>
#include <stdbool.h> 

#include "V3.h"
#include "V3_cilk.h"
#include "V3_openmp.h"

#include "V4.h"
#include "V4_openmp.h"
#include "V4_cilk.h"
#include "V4_pthreads.h"

#include "mmio.c"
#include "coo2csc.h"

int *I, *J;                     //to store the COO matrix
int M, N, nz;                   //the number of rows, columns and non-zeros of the Matrix


int comp (const void * elem1, const void * elem2) 
{
    int f = *((int*)elem1);
    int s = *((int*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

//To read the .mtx file
int read_mat(int argc, char* argv[]) {
    
    int ret_code;
    MM_typecode matcode;
    FILE *f;
    int i;
    int *val;

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
    val = (int *) malloc(nz * sizeof(int));


    /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
    /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
    /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */

    /* Replace missing val column with 1s and change the fscanf to match patter matrices*/

    if (!mm_is_pattern(matcode))
    {
    for (i=0; i<nz; i++)
    {
        int _ = fscanf(f, "%d %d %d\n", &I[i], &J[i], &val[i]);
        I[i]--;  /* adjust from 1-based to 0-based */
        J[i]--;
    }
    }
    else
    {
    for (i=0; i<nz; i++)
    {
        int _ = fscanf(f, "%d %d\n", &I[i], &J[i]);
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


int main(int argc, char* argv[]) {
    
    //Read the given Matrix
    read_mat(argc, argv);

    //If not Square Matrix return 1
    if(M != N) exit(1);
    //If not Adjacency Matrix return 2
    for(int i=0; i<nz; i++) if(I[i] == J[i]) exit(2);

    //In order to convert the Triangular Matrix into Symmetric
    int * I_ = (int *)malloc(2*nz * sizeof(int));
    int * J_ = (int *)malloc(2*nz * sizeof(int));

    //Add the symmetric elements in the COO format
    for(int i=0; i<nz; i++){
        I_[i] = I[i];
        I_[nz + i] = J[i];

        J_[i] = J[i];        
        J_[nz + i] = I[i];
    }

    //The Non-Zero Elements are doubled
    nz *= 2;
            
    //The Vetrices of Compressed Sparse Column
    uint32_t * csc_row = (uint32_t *)malloc(nz     * sizeof(uint32_t));
    uint32_t * csc_col = (uint32_t *)malloc((M + 1) * sizeof(uint32_t));

    //The Conversion from COO to CSC
    coo2csc(csc_row, csc_col,
            (uint32_t *)I_, (uint32_t *)J_,
            nz, M,
            0);

    //Sorting the CSC Matrix
    for(int i=0; i<M; i++){
        qsort ((csc_row+csc_col[i]), csc_col[i+1] - csc_col[i], sizeof(int), comp);
    }

    //Cleanup the COO variables
    free(I);
    free(J);
    free(I_);
    free(J_);

    //Getting the number of threads
    int num_threads = 8;
    if(argc < 3) {
        printf("Running with the default number of threads\n");
    }else{
        num_threads = atoi(argv[2]);
        if(num_threads <= 0){
            printf("Wrong Input\nUsage: %s [martix-market-filename] [num-of-threads]\n", argv[0]);
            return -1;
        }
        printf("Running with %d threads\n", num_threads);
        //Setting the number of threads
        omp_set_num_threads(num_threads);
                
        __cilkrts_end_cilk();
        __cilkrts_set_param("nworkers",argv[2]);
        __cilkrts_init();
    }

    //The Vertex of nodes that stores the triangles
    float * c = (float *) malloc(M * sizeof(float));
    int * c_ = (int *) malloc(M * sizeof(int));


    //Running V3
    long v3_time = v3(  csc_row, csc_col,
                        c_, M);

    int v3_sum = 0;
    for(int i=0; i<M; i++) v3_sum += c_[i];
    printf("V3: %ld us or %f s\nTriangles: %d\n\n", v3_time, (float)v3_time*1e-6, v3_sum);


    //Running V3 with Cilk
    long v3_cilk_time = v3_cilk(    csc_row, csc_col,
                                    c_, M);

    int v3_cilk_sum = 0;
    for(int i=0; i<M; i++) v3_cilk_sum += c_[i];
    printf("Cilk V3: %ld us or %f s\nTriangles: %d\n\n", v3_cilk_time, (float)v3_cilk_time*1e-6, v3_cilk_sum);


    //Running V3 with OpenMP
    long v3_omp_time = v3_cilk( csc_row, csc_col,
                                c_, M);

    int v3_omp_sum = 0;
    for(int i=0; i<M; i++) v3_omp_sum += c_[i];
    printf("OpenMP V3: %ld us or %f s\nTriangles: %d\n\n", v3_omp_time, (float)v3_omp_time*1e-6, v3_omp_sum);


    //Running V4 
    long v4_time = v4(  csc_row, csc_col,
                        c, M, nz);

    int v4_sum = 0;
    for(int i=0; i<M; i++) v4_sum += (int)c[i];
    printf("V4: %ld us or %f s\nTriangles: %d\n\n",v4_time, (float)v4_time*1e-6,v4_sum);


    //Running V4 with Cilk
    long v4_cilk_time = v4_cilk( csc_row, csc_col,
                            c, M, nz);

    int v4_cilk_sum = 0;
    for(int i=0; i<M; i++) v4_cilk_sum += (int)c[i];
    printf("Cilk V4: %ld us or %f s\nTriangles: %d\n\n", v4_cilk_time, v4_cilk_time*1e-6, v4_cilk_sum);


    //Running V4 with OpenMP
    long v4_omp = v4_openmp(    csc_row, csc_col,
                                c, M, nz);

    int v4_omp_sum = 0;
    for(int i=0; i<M; i++) v4_omp_sum += (int)c[i];
    printf("OpenMP V4: %ld us or %f s\nTriangles: %d\n\n", v4_omp, (float)v4_omp*1e-6, v4_omp_sum);
    

    //Running V4 with Pthreads
    long v4_threads = v4_pthreads(  csc_row, csc_col,
                                    c, M, nz, num_threads);

    int v4_threads_sum = 0;
    for(int i=0; i<M; i++) v4_threads_sum += (int)c[i];
    printf("PThreads V4: %ld us or %f s\nTriangles: %d\n\n", v4_threads, v4_threads*1e-6, v4_threads_sum);
    
    //Cleanup the rest of the variables
    free(csc_row);
    free(csc_col);
    free(c);
    free(c_);

    return 0;    
}
