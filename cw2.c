#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define LEN (128)
#define RMAX 10.00   /* maximum random number */
#define ROOT 0
int arrLen;
double fRand(double max);
void print2DArr(double *arr, int len);
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
    double precision = (argv[2]) ? atof(argv[2]) : 1.0;


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

    int startRow, endRow;
    for (int i = 0; i <= myrank; ++i)
    {
        endRow += rowNums[i];
    }
    startRow = endRow - rowNums[myrank];
    endRow++;

    /* init array */
    //double randArr[arrLen * arrLen];
    double *randArr = (double *)malloc(arrLen*arrLen*sizeof(double));
    if (myrank == ROOT)
    {
        for(int i = 0; i < arrLen*arrLen; i++)
        {
            randArr[i] = fRand(RMAX);
        }
        print2DArr(randArr, arrLen);
    }

    namelen = LEN;
    MPI_Get_processor_name(name, &namelen);

    
    /* do averaging */
    MPI_Bcast(randArr, arrLen*arrLen, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    int rowNum = rowNums[myrank];//endRow - startRow - 1;
    int colNum = arrLen - 2;
    double *result = (double *)malloc(size[myrank] * sizeof(double));

    averaging(randArr, startRow, endRow, result);
    for(int r = 0; r < rowNum; ++r)
    {
        if (r == 0)
        {
            printf("rank %d: %d - %d\n", myrank, startRow, endRow);
        }

        for(int c = 0; c < colNum; ++c)
        {
            printf("%.1f\t", result[r*colNum+c]);
        }
        printf("\n");
    }

    if(myrank == ROOT)
    {
    }



    MPI_Barrier(MPI_COMM_WORLD);
    
    MPI_Finalize();
    return 0;
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
void print2DArr(double *arr, int len)
{
    for(int r = 0; r < len; r++)
    {
        for(int c = 0; c < len; c++)
        {
            printf("%.1f\t", arr[r*len + c]);
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

