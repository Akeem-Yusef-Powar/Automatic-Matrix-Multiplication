#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mat.h"


int autoMmult (double *a, int arows, int acols, double *b, int brows, int bcols,double *c_calc) 
{
	mmult(c_calc, a, arows, acols, b, brows, bcols);
	return 1;

}

int autoMmultOpt (double *a, int arows, int acols, double *b, int brows, int bcols,double *c_calc) 
{
	mmultOpt(c_calc, a, arows, acols, b, brows, bcols);
	return 1;

}



int main(int argc,char *argv[]) {

	int MAT_SIZE = 200;//atoi(argv[1]); pass single in matrix size in commandline
	clock_t start,stop; // time mmult cpu burst
	double timeTaken; // cpu burst loggable value

// open log file
	FILE *Algo1Data = fopen("Algo1Time.txt","w");
	FILE *Algo2Data = fopen("Algo2Time.txt","w");

	if(Algo1Data == NULL || Algo2Data == NULL){
		printf("File/s not opened\n");
		return 1;
	}

	for(int i = MAT_SIZE; i <=2000;){ // make a matrix from size 1 to MAT_SIZE and see speeds with algo

		double *a = gen_matrix(i,i);
		double *b = gen_matrix(i,i); // make matrix of that size & auto fill rand values
        	double *c_calc = malloc(i * i * sizeof(double)); // store results

		printf("Matrix size = %d\n",i);

		start = clock();	
   		autoMmult(a, i, i, b, i, i, c_calc);
        	stop = clock();
		timeTaken = ((double)(stop-start)/CLOCKS_PER_SEC)*1000;
		//timeTaken = timeTaken/10; // get avg time taken
		printf("%f milliseconds\n\n",timeTaken);
		fprintf(Algo1Data,"%d	%.2f\n",i,timeTaken);

		// start second algo timing
		start = clock();	

   		autoMmultOpt(a, i, i, b, i, i, c_calc);
       		stop = clock();
		timeTaken = ((double)(stop-start)/CLOCKS_PER_SEC)*1000;
		//timeTaken = timeTaken/10; get avg time taken
		printf("%f milliseconds\n\n",timeTaken);
		fprintf(Algo2Data,"%d	%.2f\n",i,timeTaken);


		i=i+200;
    		free(a);
    		free(b);
    		free(c_calc);
	}
fclose(Algo1Data);
fclose(Algo2Data);
}
