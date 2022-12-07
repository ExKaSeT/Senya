#include <stdio.h>
#include <stdlib.h>


void prMatrix(float **matrix, int strs, int cols) {
    int i, j;
    for (i = 0; i < strs; i++) {
        for (j = 0; j < cols; j++) {
            printf("%.2f ", matrix[i][j]);
        }
        putchar('\n');
    }
}

int genMatrix(float ***matrix, int strs, int cols) {
    if (!((*matrix) = (float **) malloc(sizeof(float *) * strs))) {
        return 1;
    }
    int i, j, number;
    for (i = 0; i < strs; i++) {
        if (!((*matrix)[i] = (float *) malloc(sizeof(float) * cols))) {
            free((*matrix));
            return 1;
        }
        for (j = 0; j < cols; j++) {
            number = rand() % 201 - 100;
            (*matrix)[i][j] = number;
        }
    }
    return 0;
}

int multMatrixes(float **matrix1, float **matrix2, float ***result,
                 int str1, int col1, int str2, int col2, int *resultStr, int *resultCol) {
    if (!((*result) = (float **) malloc(sizeof(float *) * str1))) {
        return 1;
    }
    int i, j, n;
    if (col1 != str2) {
        return 2;
    }
    (*resultStr) = str1;
    (*resultCol) = col2;
    for (i = 0; i < str1; i++) {
        if (!((*result)[i] = (float *) malloc(sizeof(float) * (*resultCol)))) {
            return 1;
        }
        for (j = 0; j < col2; j++) {
            (*result)[i][j] = 0;
            for (n = 0; n < col1; n++) {
                (*result)[i][j] += matrix1[i][n] * matrix2[n][j];
            }
        }
    }
    return 0;
}

int findMatrixDeter(float **matrix, int str, int col, double *determinant) {
    if (str != col) {
        return 2;
    }
    if (str == 1) {
        (*determinant) = matrix[0][0];
        return 0;
    }
    (*determinant) = 1;
    int i, j, k;
    double mult, makeNull;
    for (j = 0; j < col - 1; j++) {
        for (i = 1 + j; i < str; i++) {
            if (matrix[i][j] != 0) {
                mult = (double) abs(matrix[i][j]) / abs(matrix[j][j]);
                for (k = col - 1; k >= 0; k--) {
                    makeNull = matrix[j][k] * mult;
                    if (matrix[i][j] * matrix[j][j] >= 0) {
                        matrix[i][k] -= makeNull;
                    } else {
                        matrix[i][k] += makeNull;
                    }
                }
                putchar('\n');
                prMatrix(matrix, str, col);
                putchar('\n');
            }
        }
        (*determinant) *= matrix[j][j];
    }
    (*determinant) *= matrix[j][j];
    return 0;
}

void freeMatrix(float ***matrix, int strs) {
    int i;
    for (i = 0; i < strs; i++) {
        free((*matrix)[i]);
    }
    free((*matrix));
}

int main() {
    int str1 = 2, str2 = 2, col1 = 2, col2 = 2;
    int strRes, colRes;
    float **matrix1, **matrix2, **multResult;
    double deter1 = 0, deter2 = 0;

    int statusCode = genMatrix(&matrix1, str1, col1);
    if (statusCode != 0) {
        return statusCode;
    }
    statusCode = genMatrix(&matrix2, str2, col2);
    if (statusCode != 0) {
        freeMatrix(&matrix1, str1);
        return statusCode;
    }
    printf("First matrix:\n");
    prMatrix(matrix1, str1, col1);
    printf("\nSecond matrix:\n");
    prMatrix(matrix2, str2, col2);
    putchar('\n');

    statusCode = multMatrixes(matrix1, matrix2, &multResult, str1, col1, str2, col2, &strRes, &colRes);
    if (statusCode != 0) {
        freeMatrix(&matrix1, str1);
        freeMatrix(&matrix2, str2);
        return statusCode;
    }
    printf("The result of matrixes multypling:\n");
    prMatrix(multResult, strRes, colRes);
    putchar('\n');

    statusCode = findMatrixDeter(matrix1, str1, col1,&deter1);
    if (statusCode != 0) {
        freeMatrix(&matrix1, str1);
        freeMatrix(&matrix2, str2);
        freeMatrix(&multResult, strRes);
        return statusCode;
    }
    statusCode = findMatrixDeter(matrix2, str2, col2, &deter2);
    if (statusCode != 0) {
        freeMatrix(&matrix1, str1);
        freeMatrix(&matrix2, str2);
        freeMatrix(&multResult, strRes);
        return statusCode;
    }

    printf("Determinants of 1st matrix: %.6lf\t 2nd matrix: %.6lf\n", deter1, deter2);

    freeMatrix(&matrix1, str1);
    freeMatrix(&matrix2, str2);
    freeMatrix(&multResult, strRes);
    return 0;
}