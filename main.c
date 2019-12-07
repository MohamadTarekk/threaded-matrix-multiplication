#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

///Structs
typedef struct MATRIX
{
    int **data;
    int rows;
    int columns;
    double elapsedTime;
} MATRIX;
typedef struct THREAD_ARGUMENT
{
    MATRIX A,B;
    MATRIX *result;// = (MATRIX *) malloc(sizeof(MATRIX));
    int row;
    int column;
} THREAD_ARGUMENT;


///Functions Prototypes
/**Reading matrices*/
MATRIX* readMatrices();
int** fillMatrix(FILE *f, int rows, int columns);
void writeOutput(MATRIX *matrices);
void printMatrix(MATRIX matrix, char name);
/**Matrix multiplication*/
// Serial calculation
void matrixMultiplication(MATRIX A, MATRIX B, MATRIX *C);
// Element by element threading
void evaluateElementByElement(MATRIX A, MATRIX B, MATRIX *C);
void* evaluateElementThread(void *ptr);
// Row by row threading
void evaluateRowByRow(MATRIX A, MATRIX B, MATRIX *C);
void* evaluateRowThread(void *ptr);
// Element evaluation
void calculateElement(MATRIX A, MATRIX B, MATRIX* C, int row, int column);
/**Mainmenu*/
void mainmenu();

int main()
{
    mainmenu();
    return 0;
}

/**Reading matrices*/
MATRIX* readMatrices()
{
    // matrices array
    MATRIX  *matrices = (MATRIX *) malloc(2 * sizeof(MATRIX));
    // open the file for reading;
    FILE *f = fopen("input.txt", "r");
    // fill the matrice
    fscanf(f, "%d %d\n", &matrices[0].rows, &matrices[0].columns);
    matrices[0].data = fillMatrix(f, matrices[0].rows, matrices[0].columns);
    fscanf(f, "%d %d\n", &matrices[1].rows, &matrices[1].columns);
    matrices[1].data = fillMatrix(f, matrices[1].rows, matrices[1].columns);
    fclose(f);
    // print matrices
    //printMatrix(matrices[0], 'A');
    //printf("\n");
    //printMatrix(matrices[1], 'B');

    return matrices;
}

int** fillMatrix(FILE *f, int rows, int columns)
{
    int **matrix = (int**) malloc(rows * sizeof(int *));
    int *newRow;
    int i,j;
    for (i = 0; i < rows; i++)
    {
        newRow = (int*) malloc(columns * sizeof(int));
        for(j = 0; j < (columns-1) ; j++)
        {
            fscanf(f, "%d ", &newRow[j]);
        }
        fscanf(f, "%d\n", &newRow[j]);
        matrix[i] = newRow;
    }
    return matrix;
}

void writeOutput(MATRIX *matrices)
{
    FILE *f = fopen("output.txt", "w");
    int count, row, column;
    for (count = 0; count < 3; count++)
    {
        fprintf(f, "%d\n", (matrices[count].rows * matrices[count].columns));
        for (row = 0; row < matrices[count].rows; row++)
        {
            for (column = 0; column < matrices[count].columns; column++)
                fprintf(f, "%d ", matrices[count].data[row][column]);
            fprintf(f, "\n");
        }
        fprintf(f, "Elapsed time: %.0lf micro sec.\n", matrices[count].elapsedTime);
    }
    fclose(f);
}

void printMatrix(MATRIX matrix, char name)
{
    int rows = matrix.rows;
    int columns = matrix.columns;
    printf("matrix %c (size: %d x %d)\n", name, rows, columns);
    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < columns; j++)
            printf("%d\t", matrix.data[i][j]);
        printf("\n");
    }
    return;
}

/**Matrix multiplication*/
// Serial calculation
void matrixMultiplication(MATRIX A, MATRIX B, MATRIX *C)
{
    (*C).rows = A.rows;
    (*C).columns = B.columns;
    (*C).data = (int**) malloc(A.rows * sizeof(int*));
    int x, y;
    for (x = 0; x < A.rows; x++)
    {
        (*C).data[x] = (int*) malloc(B.columns * sizeof(int));
        for (y = 0; y < B.columns; y++)
        {
            calculateElement(A, B, C, x, y);
        }
    }
    return;
}

// Element by element threading
void evaluateElementByElement(MATRIX A, MATRIX B, MATRIX *C)
{
    (*C).rows = A.rows;
    (*C).columns = B.columns;
    (*C).data = (int**) malloc(A.rows * sizeof(int*));
    int x, y;
    int z = 0;
    int threadsCount = A.rows * B.columns;
    pthread_t threads[threadsCount];
    for (x = 0; x < A.rows; x++)
    {
        (*C).data[x] = (int*) malloc(B.columns * sizeof(int));
        for (y = 0; y < B.columns; y++)
        {
            // create & initialize thread struct
            THREAD_ARGUMENT *args = (THREAD_ARGUMENT*) malloc(sizeof(THREAD_ARGUMENT));
            (*args).A = A; (*args).B = B; (*args).result = C; (*args).row = x;  (*args).column = y;
            // create thread
            pthread_create(&threads[z], NULL, evaluateElementThread, (void*) args);
            z++;
        }
    }
    for(z = 0; z < threadsCount; z++)
    {
        pthread_join(threads[z], NULL);
    }
    return;
}

void* evaluateElementThread(void *ptr)
{
    THREAD_ARGUMENT data = *((THREAD_ARGUMENT *) ptr);

//    calculateElement(data.A, data.B, data.result, data.row, data.column);
    (*data.result).data[data.row][data.column] = 0;
    int i;
    for (i = 0; i < data.A.columns; i++)
    {
        (*(data.result)).data[data.row][data.column] += data.A.data[data.row][i] * data.B.data[i][data.column];
    }

    return NULL;
}

// Row by row threading
void evaluateRowByRow(MATRIX A, MATRIX B, MATRIX *C)
{
    (*C).rows = A.rows;
    (*C).columns = B.columns;
    (*C).data = (int**) malloc(A.rows * sizeof(int*));
    int x;
    int threadsCount = A.rows;
    pthread_t threads[threadsCount];
    for (x = 0; x < A.rows; x++)
    {
        (*C).data[x] = (int*) malloc(B.columns * sizeof(int));
        THREAD_ARGUMENT *args = (THREAD_ARGUMENT *) malloc(sizeof(THREAD_ARGUMENT));
        (*args).A = A; (*args).B = B; (*args).result = C; (*args).row = x;
        pthread_create(&threads[x], NULL, evaluateRowThread, (void*) args);
    }
    for(x = 0; x < threadsCount; x++)
    {
        pthread_join(threads[x], NULL);
    }
    return;
}

void* evaluateRowThread(void *ptr)
{
    THREAD_ARGUMENT args = *((THREAD_ARGUMENT *) ptr);
    int i, j;
    for (i = 0; i < args.B.columns; i++)
    {
        (*args.result).data[args.row][i] = 0;
        for (j = 0; j < args.B.rows; j++)
        {
            (* args.result).data[args.row][i] += args.A.data[args.row][j] * args.B.data[j][i];
        }
    }
    return NULL;
}

// Element evaluation
void calculateElement(MATRIX A, MATRIX B, MATRIX* C, int row, int column)
{
    (*C).data[row][column] = 0;
    int i;
    for (i = 0; i < A.columns; i++)
    {
        (*C).data[row][column] += A.data[row][i] * B.data[i][column];
    }
    return;
}

/**Mainmenu*/
void mainmenu()
{
    printf("Reading input data...\n\n");
    MATRIX *matrices = readMatrices();
    if(matrices[0].columns != matrices[1].rows)
    {
        printf("Math Error: Size mismatch!\n");
        exit(0);
    }
    printf("\nReading completed successfully\n");
    printf("\n**************************************************************\n");
    // get results
    MATRIX* result = (MATRIX *)malloc(3 * sizeof(MATRIX));
    clock_t startTime, endTime;
    double elapsedTime;
    // normal matrix multiplication
    printf("\nMatrix multiplication using no threads\n");
    startTime = clock();
    matrixMultiplication(matrices[0], matrices[1], &result[0]);
    endTime = clock();
    elapsedTime = 1000000.0 * ((double) (endTime - startTime)) / CLOCKS_PER_SEC;
    result[0].elapsedTime = elapsedTime;
    //printMatrix(result[0], 'X');
    printf("Elapsed time: %.0lf micro sec.\n", elapsedTime);
    // threading each for each element in the resulting matrix
    printf("\nMatrix multiplication using a thread for each elememt\n");
    startTime = clock();
    evaluateElementByElement(matrices[0], matrices[1], &result[1]);
    endTime = clock();
    elapsedTime = 1000000.0 * ((double) (endTime - startTime)) / CLOCKS_PER_SEC;
    result[1].elapsedTime = elapsedTime;
    //printMatrix(result[1], 'Y');
    printf("Elapsed time: %.0lf micro sec.\n", elapsedTime);
    // threading for each row in the resulting matrix
    printf("\nMatrix multiplication using a thread for each row\n");
    startTime = clock();
    evaluateRowByRow(matrices[0], matrices[1], &result[2]);
    endTime = clock();
    elapsedTime = 1000000.0 * ((double) (endTime - startTime)) / CLOCKS_PER_SEC;
    result[2].elapsedTime = elapsedTime;
    //printMatrix(result[2], 'Z');
    printf("Elapsed time: %.0lf micro sec.\n", elapsedTime);
    // write to output file
    writeOutput(result);
    return;
}
