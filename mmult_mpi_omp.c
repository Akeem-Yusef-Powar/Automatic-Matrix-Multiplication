/** 
 * Incomplete program to multiply a matrix times a matrix using both
 * MPI to distribute the computation among nodes and OMP
 * to distribute the computation among threads.
 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#define min(x, y) ((x)<(y)?(x):(y))

#include "mat.h"


int mmult_omp(double *c,
	      double *a, int aRows, int aCols,
	      double *b, int bRows, int bCols){

  int i, j, k;

#pragma omp parallel default(none)       \
  shared(a, b, c, aRows, aCols, bRows, bCols) private(i, k, j)
#pragma omp for
    for (i =0; i < aRows; i++){
      for (j=0; j < bCols; j++){
	c[i*bCols + j] = 0;
      }
      for (k = 0; k < aCols; k++){
	for (j=0; j< bCols; j++){
	  c[i*bCols + j] += a[i*aCols + k] * b[k*bCols + j];
	}
      }
    }
  return 0;

}





int main(int argc, char* argv[])
{
    int nrows, ncols;
    double *aa;	/* the A matrix */
    double *bb;	/* the B matrix */
    double *cc1;	/* A x B computed using the omp-mpi code you write */
    double *cc2;	/* A x B computed using the conventional algorithm */
    int myid, numprocs;
    double starttime, endtime;
    MPI_Status status;


    int numberSent = 0;
    int main = 0;
    int sent = 0;
    int row = 0;
    int setNumber = 0;
    double *buff;
    double *res;
    

    /* insert other global variables here */

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    if (argc > 1) {
        nrows = atoi(argv[1]);
        ncols = nrows;
        
        if (myid == 0) {
            // Master Code goes here
            aa = gen_matrix(nrows, ncols);
            bb = gen_matrix(ncols, nrows);
            cc1 = malloc(sizeof(double) * nrows * nrows);
	    buff = (double*)malloc(sizeof(double) * ncols);
	    res = (double*)malloc(sizeof(double) *nrows);
	    starttime = MPI_Wtime();

	    MPI_Bcast(bb, nrows * ncols, MPI_DOUBLE, main, MPI_COMM_WORLD);

	    /* Insert your master code here to store the product into cc1 */
	    
	    for(int i = 0; i < min(numprocs-1, nrows); i++){
	      for(int j =0; j <ncols; j++){
		buff[j] = aa[i * ncols + j];
	      }

	      MPI_Send(buff, ncols, MPI_DOUBLE, i+1, ++numberSent, MPI_COMM_WORLD);
	    }
	    
	    for( int i = 0; i<nrows; i++){
	      MPI_Recv(res, nrows, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	      sent = status.MPI_SOURCE;

	      row = status.MPI_TAG;

	      for(int l= 0 ; l<ncols; l++){
		cc1[row*nrows + 1] = res[l];
	      }

	      if(numberSent < nrows){
		for(int l =0; l < nrows; l++){
		  buff[l] = aa[setNumber*nrows +1];
		}
	      

	      MPI_Send(buff, ncols, MPI_DOUBLE, sent, ++setNumber, MPI_COMM_WORLD);
	      }
	
	      
	 else{
	      MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, sent, ++setNumber, MPI_COMM_WORLD);
	    }
	
	    }
	   
            endtime = MPI_Wtime();
            printf("%f\n",(endtime - starttime));
            cc2  = malloc(sizeof(double) * nrows * nrows);
            mmult(cc2, aa, nrows, ncols, bb, ncols, nrows);
            compare_matrices(cc2, cc1, nrows, nrows);
	} else {
	      // Slave Code goes here

	  MPI_Bcast(bb, nrows*ncols, MPI_DOUBLE, main, MPI_COMM_WORLD);

	  if(myid <- nrows){
	    while(1){
	      MPI_Recv(buff, ncols, MPI_DOUBLE, main, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	      if(status.MPI_TAG == 0){
		break;
	      }

	      row = status.MPI_TAG - 1;


		mmult_omp(res, buff, 1, ncols, bb, nrows, ncols);

	      MPI_Send(res, nrows, MPI_DOUBLE, main, row, MPI_COMM_WORLD);
	    }
	  }


	  
        }
    } else {
        fprintf(stderr, "Usage matrix_times_vector <size>\n");
    }
	    
    MPI_Finalize();
    return 0;
}
