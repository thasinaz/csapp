/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void trans(int M, int N, int A[N][M], int B[M][N]);
void transpose32_32(int A[32][32], int B[32][32]);
void transpose64_64(int A[64][64], int B[64][64]);
void transpose61_67(int A[61][67], int B[67][61]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32 && N == 32) {
        transpose32_32(A, B);
    } else if (M == 64 && N == 64) {
        transpose64_64(A, B);
    } else if (M == 61 && N == 67) {
        transpose61_67(A, B);
    } else {
        trans(M, N, A, B);
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

void transpose32_32(int A[32][32], int B[32][32])
{
    for (int i = 0; i < 32; i += 8) {
        for (int j = 0; j < 32; j += 8) {
            int contain_diagonal = i == j;
            for (int k = i; k < i + 8; k++) {
                for (int l = j; l < j + 8; l++) {
                    if (k != l) {
                        B[l][k] = A[k][l];
                    }
                }
                if (contain_diagonal) {
                    B[k][k] = A[k][k];
                }
            }
        }
    }
}

void transpose64_64(int A[64][64], int B[64][64])
{
    for (int i = 0; i < 64; i += 4) {
        for (int j = 0; j < 64; j += 4) {
            for (int k = i; k < i + 4; k++) {
                int v0, v1, v2, v3;
                v0 = A[k][j];
                v1 = A[k][j + 1];
                v2 = A[k][j + 2];
                v3 = A[k][j + 3];
                B[j][k] = v0;
                B[j + 1][k] = v1;
                B[j + 2][k] = v2;
                B[j + 3][k] = v3;
            }
        }
    }
}

void transpose61_67(int A[61][67], int B[67][61])
{
    for (int i = 0; i < 61; i += 17) {
        for (int j = 0; j < 67; j += 17) {
            for (int k = i; k < i + 17 && k < 61; k++) {
                for (int l = j; l < j + 17 && l < 67; l++) {
                    B[l][k] = A[k][l];
                }
            }
        }
    }
}

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

