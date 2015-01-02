#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define LEN (128)
#define RMAX 100.00   /* maximum random number */
#define ROOT 0
int arrLen;
double precision;
int isEnd;
double fRand(double max);
void print2DArr(double *arr, int row, int col);
void replaceArr(double *arr, double *newArr, int *rowNums, int rank);
void averaging(double *arr, int row_s, int row_e, double *result);

int main(int argc, char **argv)
{
    /* get command-line arguments
     *
     * default value:
     * square array length: default: 10
     * precision: default: 1.0
     * number of threads: default: 1
     */
    arrLen = (argv[1]) ? strtol(argv[1], NULL, 10) : 10;
    precision = (argv[2]) ? atof(argv[2]) : 1.0;

    isEnd = 0;


    /* init mpi */
    int rc, myrank, nproc, namelen;
    char name[LEN];

    rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS)
    {
        printf ("Error starting MPI program\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    /* calculate number of rows to average for each node */
    int rowNums[nproc];
    int size[nproc];
    for (int i = 0; i < nproc; ++i)
    {
        rowNums[i] = (arrLen - 2) / nproc;
        if (i < (arrLen - 2) % nproc)
        {
            rowNums[i]++;
        }
        size[i] = rowNums[i] * (arrLen - 2);
    }

    int startRow = 0;
    for (int i = 0; i < myrank; ++i)
    {
        startRow += rowNums[i];
    }
    int endRow = startRow + rowNums[myrank] + 1;
    //printf("rank: %d: %d - %d\n", myrank, startRow, endRow);

    /* init array */
    double randArr[arrLen * arrLen];
    if (myrank == ROOT)
    {
        for(int i = 0; i < arrLen*arrLen; i++)
        {
            randArr[i] = fRand(RMAX);
        }
        //printf("Initial square array:\n");
        //print2DArr(randArr, arrLen, arrLen);
    }

    namelen = LEN;
    MPI_Get_processor_name(name, &namelen);
    printf("rank: %d in node: %s\n", myrank, name);

    
    


    /* send and receive result */
    int counter = 1;
    while(!isEnd)
    {
        isEnd = 1;
        /* do averaging */
        MPI_Bcast(randArr, arrLen*arrLen, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        double result[size[myrank]];
        averaging(randArr, startRow, endRow, result);
        //print2DArr(result, rowNums[myrank], arrLen - 2);
        if(myrank == ROOT)
        {
            replaceArr(randArr, result, rowNums, 0); 

            for(int i = 1; i < nproc; ++i)
            {
                double newArr[size[i]];
                MPI_Status stat;
                MPI_Recv(newArr, size[i], MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &stat);
                //print2DArr(newArr, rowNums[i], arrLen - 2);
                replaceArr(randArr, newArr, rowNums, i); 
            }
            //printf("Round %d:\n", counter);
            //print2DArr(randArr, arrLen, arrLen);
            counter++;
        }
        else
        {
            MPI_Send(result, size[myrank], MPI_DOUBLE, ROOT, 0, MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(&isEnd, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    //printf("rank: %d finished.\n", myrank);
    
    MPI_Finalize();

    return 0;
}

/*
 * replaceArr: replace old values with new values with regard to rank number
 */
void replaceArr(double *arr, double *newArr, int *rowNums, int rank)
{
    int row_s = 0;
    for (int i = 0; i < rank; ++i)
    {
        row_s += rowNums[i];
    }
    int row_e = row_s + rowNums[rank] + 1;

    int col = arrLen;
    int i = 0;
    for(int r = row_s+1; r < row_e ; ++r)
    {
        for(int c = 1; c < col - 1; ++c)
        {
            if(fabs(arr[r*col + c] - newArr[i]) >= precision)  isEnd = 0;
            arr[r*col + c] = newArr[i];
            i++;
        }
    }
    //print2DArr(arr, arrLen, arrLen);
}


/*
 * averaging: replacing a value with the average of its four neighbours
 * return a new array after averaging
 */
void averaging(double *arr, int row_s, int row_e, double *result)
{
    int col = arrLen;
    int i = 0;
    for(int r = row_s+1; r < row_e ; ++r)
    {
        for(int c = 1; c < col - 1; ++c)
        {
            result[i] = (arr[r*col + c - 1] + arr[r*col + c + 1] +
                    arr[(r-1)*col + c] + arr[(r+1)*col +c]) / 4;
            i++;
        }
    }
}


/*
 * print2DArr: print a readable 2D array
 */
void print2DArr(double *arr, int row, int col)
{
    for(int r = 0; r < row; r++)
    {
        for(int c = 0; c < col; c++)
        {
            printf("%3.2f\t", arr[r*col + c]);
        }
        printf("\n");
    }

}

/*
 * fRand: return random double with 2 decimal places
 */
double fRand(double max)
{
    double f = (double)rand() / RAND_MAX;
    return f * max;
}

