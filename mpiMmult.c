#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#define min(x, y) ((x)<(y)?(x):(y))
#include "mat.h"


int main (int argc, char* argv[])
{

    int id,numProcess,whoSent,index;
    char name[255];
    int MAT_SIZE = atoi(argv[1]);
    double *a, *b, *c_calc, *workLoad;
    int numSent = 0; 
    MPI_Status status;
    double *result;
    clock_t start,stop;
    double timeTaken;
	int done = 0; 

    MPI_Init(&argc,&argv); // first execuatble statment
    MPI_Comm_size(MPI_COMM_WORLD, &numProcess); // find out how many processes we have
    MPI_Comm_rank(MPI_COMM_WORLD, &id); // find out their id
    gethostname(name,254);
    //printf("Process %s ready. ID %d/%d\n", name, id, numProcess);

    a = malloc(MAT_SIZE * MAT_SIZE * sizeof(double)); // every process malloc's space for arrays fixes null pointer issues with MPI_Recv
    b = malloc(MAT_SIZE * MAT_SIZE * sizeof(double));
    c_calc = malloc(MAT_SIZE * MAT_SIZE * sizeof(double)); // result array for controller
    result = malloc(sizeof(double)* MAT_SIZE); // row of result and do 1/MAT_SIZE calc vs MAT_SIZE^2
    workLoad = malloc(MAT_SIZE * sizeof(double)); // for passing to workers 1 * col slice


    if(id == 0)  // controller behaviour
    {
        a = gen_matrix(MAT_SIZE,MAT_SIZE);
        b = gen_matrix(MAT_SIZE,MAT_SIZE); // only controller makes arrays

	start = clock();

        MPI_Bcast(b, MAT_SIZE*MAT_SIZE,MPI_DOUBLE, 0, MPI_COMM_WORLD); // share b

        for(int sendToWorker = 0; sendToWorker < min(numProcess-1,MAT_SIZE); sendToWorker++) // for each worker or  row in a
        {
            for(int j = 0; j < MAT_SIZE; j++)  // each row slice is MAT_SIZE elements
            {
                workLoad[j] = a[numSent*MAT_SIZE+j]; // put row of a in buffer
                //printf(" con wl[%d] %3lf \n", j,workLoad[j]);
            }
            //         buffer,amount of items,type,destination id,tag,something
            MPI_Send(workLoad,MAT_SIZE,MPI_DOUBLE,sendToWorker+1,numSent,MPI_COMM_WORLD);// send buffer to worker
	    numSent = numSent+1; // keep track of what index row result goes in
            //printf("processing c_calc index %d \n",numSent);
        }

        for(int waitForWorker = 0; waitForWorker < MAT_SIZE; waitForWorker++) // recv one row and send one row if needed
        {
		if(done == 2 && MAT_SIZE > 404){break;} //if MAT_SIZE > 404 there are finish errors, this is a work around 
            MPI_Recv(result,MAT_SIZE,MPI_DOUBLE,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);// workers return single row * col result
            whoSent = status.MPI_SOURCE; // note who sent result
            //printf("recved from %d \n",whoSent);

            index = status.MPI_TAG; // what row index of c_calc
            for(int add = 0; add < MAT_SIZE ; add++) // add row recv to c_calc
            {
                c_calc[index*waitForWorker+add] = result[add];
            }
            if(numSent < MAT_SIZE)  // if cols left to send
            {
                for(int g = 0; g < MAT_SIZE ; g++)
                {
                    workLoad[g] = a[numSent*MAT_SIZE + g];
		    //printf(" con wl[%d] %3lf \n", g,workLoad[g]);		
                }
                MPI_Send(workLoad,MAT_SIZE,MPI_DOUBLE,whoSent,numSent,MPI_COMM_WORLD); // send another workLoad
                numSent = numSent+1; // keep track of what index result goes in
            }
            else
            {
                MPI_Send(MPI_BOTTOM,0,MPI_DOUBLE,whoSent,404,MPI_COMM_WORLD); //404, work not found so quit
		done = done + 1;
            }

        }
	stop = clock();
	timeTaken = ((double)(stop-start)/CLOCKS_PER_SEC)*1000;
	printf("time taken %lf \n",timeTaken);
        FILE *log = fopen("mpiTiming.txt","a");
	if(log == NULL){
		printf("time not logged\n");
	} else {
		fprintf(log,"%d,	%3lf\n",MAT_SIZE,timeTaken);
	}
	fclose(log);
    }

    // workers go here
    else
    {
        MPI_Bcast(b, MAT_SIZE*MAT_SIZE,MPI_DOUBLE, 0, MPI_COMM_WORLD);
        while(1) // always work unless told to break
        {
            MPI_Recv(workLoad,MAT_SIZE,MPI_DOUBLE,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status); // get slice
            int returnIndex = status.MPI_TAG; // what col of b to use
            if(returnIndex == 404)
            {
                printf("worker quit %d\n",id);
		//done = done +1;
                break;
            }
            mmult(result, workLoad, 1,MAT_SIZE, b, MAT_SIZE, MAT_SIZE);
            MPI_Send(result,MAT_SIZE,MPI_DOUBLE,0,returnIndex,MPI_COMM_WORLD); // results row, length,type, send to who, row index,somthing
            //printf("worker sent returned index %d\n",returnIndex);
        }
    }

	free(a);
	free(b);
	free(c_calc);
	free(workLoad);
	free(result);

    MPI_Finalize();
// finish timer
    return 0;
}
