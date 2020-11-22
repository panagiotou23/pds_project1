#include <sys/time.h>
#include <iostream>
#include "mmio.c"

#include "coo2csc.h"

using namespace std;

int *I, *J;
int M, N, nz;
double *val;

uint32_t * csc_row,
         * csc_col;

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

    for (i=0; i<nz; i++)
    {
        fscanf(f, "%d %d\n", &I[i], &J[i]);// %lg , &val[i]
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
        fprintf(stdout, "%d %d %20.19g\n", I[i], J[i], val[i]);
	return 0;
}

long find_triangle(int *c) {
    
    struct timespec ts_start;
    struct timespec ts_end;
    N=0;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    
    for(int i=0; i<M-2; i++)
        for(int j=csc_col[i]; j<csc_col[i+1]; j++)
            for(int k=csc_col[csc_row[j]]; k<csc_col[csc_row[j] + 1]; k++)
                for(int l=csc_col[i]; l<csc_col[i+1]; l++)
                    if(csc_row[k] == csc_row[l]){
                        c[i]++;
                        c[csc_row[j]]++;
                        c[csc_row[k]]++;
                        N++;
                        cout<<i<<" "<< csc_row[j]<<" "<<csc_row[k]<<endl;
                    } 

    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    return (ts_end.tv_nsec - ts_start.tv_nsec);
    cout << endl << endl;
}

int main(int argc, char* argv[]) {

    // Read Matrix
    read_mat(argc, argv);

    cout<< endl << endl;

    if(M != N) exit(1);

    for(int i=0; i<nz; i++)
        if(I[i] == J[i]) exit(1);

    csc_row = (uint32_t *)malloc(nz     * sizeof(uint32_t));
    csc_col = (uint32_t *)malloc((M + 1) * sizeof(uint32_t));

    // Call coo2csc for isOneBase false
    coo2csc(csc_row, csc_col,
            (uint32_t *)I, (uint32_t *)J,
            nz, M,
            0);

    /* cleanup variables */
    free(I);
    free(J);
    
    cout<< endl << endl;

    int *c = (int *) malloc(M * sizeof(int));
    
    for(int i=0; i<M; i++)
        c[i] = 0;

    long time = find_triangle(c);
    
    //for(int i=0; i<M; i++) cout<<c[i]<<" ";
    
    cout<<N;
    cout<<"\nV3: "<<time<<" ns"<<endl;
    
    /* cleanup variables */
    free( csc_row );
    free( csc_col );
    return 0;    
}