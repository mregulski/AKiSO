#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
// maximum value of matrix' elements - used in random fill
#ifndef VAL_MAX
    #define VAL_MAX 10000
#endif
#define EMPTY 0
#define FULL 1

typedef struct { 
    int size;
    long **matrix;
} Matrix;

typedef struct {
    Matrix *mtr1;
    Matrix *mtr2;
    Matrix *res;
    int row_num;
    int th_num;
} thrProperties;

pthread_mutex_t lock;


// Prototypes
Matrix *gen_matrix(int size, int fill);
void print_matrix(Matrix *mtr);
void free_matrix(Matrix *mtr);
long *get_result_row(Matrix *mtr1, Matrix *mtr2, int row_num);

void check(void *);
// END prototypes

void *threadLogic(void *arg)
{
    thrProperties *props = (thrProperties *) arg;
    while(props->row_num < props->res->size)
    {
        if (props->row_num >= props->mtr1->size)
                pthread_exit(NULL);
        long *row = get_result_row(props->mtr1, props->mtr2, props->row_num);
            //props->res->matrix[props->row_num] = malloc(sizeof(long *) * props->mtr1->size);
        //printf("row_num: %d row[0]: %ld\n",props->row_num,row[0]);
        props->res->matrix[props->row_num] = row;
        props->row_num += props->th_num;
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{ 
    printf("%ld",RAND_MAX);
    // Check & prepare parameters
    if (argc != 3)
    {
        printf("Usage: /path/to/matrix MATRIX_SIZE NUM_THREADS\n");
        exit(EXIT_FAILURE);
    }
    int m_size = atoi(argv[1]);
    int n_thr = atoi(argv[2]);
    // don't use more threads than there are rows to calculate
    if (n_thr > m_size)
        n_thr = m_size;
    printf("[I] size: %d | threads: %d\n", m_size, n_thr);
    
    if (m_size <= 0 || n_thr <= 0)
    {
        printf("Both arguments must be > 0\n");
        exit(EXIT_FAILURE);
    }
    // Parameters OK
    // prepare matrices
    srandom(time(0));

    Matrix *matrix_1 = gen_matrix(m_size, FULL);
    Matrix *matrix_2 = gen_matrix(m_size, FULL);
    Matrix *result = gen_matrix(m_size, EMPTY);
    printf("[I] Matrices OK\n");
    // matrices OK (checked in gen_matrix())
    // prepare threads
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "matrix: mutex initialization failed\n");
        exit(EXIT_FAILURE);
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * n_thr);
    check(threads);
    
    thrProperties **thr_props = malloc(sizeof(thrProperties *) * n_thr);
    check(thr_props);

    printf("[I] Thread properties array allocated\n");
    for (int i = 0; i <= n_thr; i++)
    {
        thr_props[i] = malloc(sizeof(thrProperties) * n_thr);
        thr_props[i]->mtr1 = matrix_1;
        thr_props[i]->mtr2 = matrix_2;
        thr_props[i]->res = result;
        thr_props[i]->row_num = i;
        thr_props[i]->th_num = n_thr;
    }
    printf("[I] Thread properties created \n");
    // calculate matrix_1 * matrix_2
    
    for (int j = 0; j <= n_thr; j++)
    {
        pthread_create(&threads[j], NULL, threadLogic, thr_props[j]);
    }
    for (int k = 0; k <= n_thr; k++)
        pthread_join(threads[k],NULL);
    print_matrix(result);
    // Clean up
    free_matrix(matrix_1);
    free_matrix(matrix_2);
    free_matrix(result);
    printf("[I] Matrices freed\n");
    pthread_mutex_destroy(&lock);
    printf("[I] Mutex lock destroyed\n");
    return 0;
 }


// Create size x size matrix, optionally filled with random values no larger than MAX_VAL
Matrix *gen_matrix(int size, int fill)
{ 
    Matrix *mtr = malloc(sizeof(Matrix *));
    check(mtr);
    mtr->size = size;
    mtr->matrix = malloc(sizeof(long **) * mtr->size);
    check(mtr->matrix);
    for (int row = 0; row < mtr->size; row++)
    {
        mtr->matrix[row] = malloc(sizeof(long) * mtr->size);
        check(mtr->matrix[row]);
        for (int col = 0; col < mtr->size; col++)
        {
            if (fill)
                mtr->matrix[row][col] = random() % VAL_MAX;
            else
                mtr->matrix[row][col] = 0;
        }
    }
    return mtr;
}

// Return row_num'th rown of mtr1 x mtr2
long *get_result_row(Matrix *mtr1, Matrix *mtr2, int row_num)
{
    long *result = malloc(sizeof(long *) * mtr1->size);
    for(int j = 0; j < mtr1->size; j++)
        for(int l = 0; l < mtr1->size; l++)
            result[j] += mtr2->matrix[l][j] * mtr1->matrix[row_num][l];
    return result;
}

// Print formatted matrix
void print_matrix(Matrix *mtr)
{
    int colwidth = floor(log10(VAL_MAX)) + 1;
    for (int row = 0; row < mtr->size; row++)
    {
        for (int col = 0; col < mtr->size; col++)
            printf("%*ld ", colwidth, mtr->matrix[row][col]);
        printf("\n");
    }
    printf("\n");
}

// deallocate matrix structure
void free_matrix(Matrix *mtr)
{
    for (int row = 0; row < mtr->size; row++)
        free(mtr->matrix[row]);
    free(mtr->matrix);
    free(mtr);
}

void check (void *ptr)
{
    if (!ptr)
    {
        perror("matrix: allocation error");
        exit(EXIT_FAILURE);
    }
}

