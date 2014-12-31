#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define LEN (128)
#define RMAX 10.00   /* maximum random number */
#define ROOT 0
int arrLen;
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
    //double precision = (argv[2]) ? atof(argv[2]) : 1.0;


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
    //double *randArr = (double *)malloc(arrLen*arrLen*sizeof(double));
    if (myrank == ROOT)
    {
        for(int i = 0; i < arrLen*arrLen; i++)
        {
            randArr[i] = fRand(RMAX);
        }
        printf("Initial square array:\n");
        print2DArr(randArr, arrLen, arrLen);
    }

    namelen = LEN;
    MPI_Get_processor_name(name, &namelen);

    
    /* do averaging */
    MPI_Bcast(randArr, arrLen*arrLen, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    //int rowNum = rowNums[myrank];//endRow - startRow - 1;
    //int colNum = arrLen - 2;
    double result[size[myrank]];
    //double *result = (double *)malloc(size[myrank] * sizeof(double));

    averaging(randArr, startRow, endRow, result);
    //print2DArr(result, rowNums[myrank], arrLen - 2);
    


    //int n[5] = {0,1,2,3,4};
    if(myrank == ROOT)
    {
        for(int i = 1; i < nproc; ++i)
        {
           //int *r = (int *)malloc(5 * sizeof(int));
          // int r[5];
          // MPI_Status stat;
          // MPI_Recv(r, 5, MPI_INT, i, 0, MPI_COMM_WORLD, &stat);
          // for(int m = 0; m < 5; ++m)
          // {
          //     printf("%d\t", r[m]);
          // }
          //printf("%d\n", i);
           //free(r); 
            //double *newArr1 = (double *)malloc(size[i] * sizeof(double));
            double newArr[size[i]];
            MPI_Status stat;
            MPI_Recv(newArr, size[i], MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &stat);
            print2DArr(newArr, rowNums[i], arrLen - 2);
            replaceArr(randArr, newArr, rowNums, i); 
            //free(newArr);
        }
        printf("Result square array:\n");
        print2DArr(randArr, arrLen, arrLen);
    }
    else
    {
        //n[0] = myrank;
        //MPI_Send(n, 5, MPI_INT, ROOT, 0, MPI_COMM_WORLD);
        MPI_Send(result, size[myrank], MPI_DOUBLE, ROOT, 0, MPI_COMM_WORLD);
    }
    




    MPI_Barrier(MPI_COMM_WORLD);
    
    MPI_Finalize();

    return 0;
}


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
            //result[i] = i;
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
            printf("%.1f\t", arr[r*col + c]);
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

