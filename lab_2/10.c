#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>


int toDecimal(char ch) {
    ch = tolower(ch);
    return isdigit(ch) ? ch - '0' : ch - 'a' + 10;
}

char toAscii(int value) {
    return (value < 10 ? value + '0' : value + 'A' - 10);
}

void inCommonFraction(int *numenator, int *denumenator, char *number) {
    int count = 0, numLen = strlen(number);
    (*numenator) = 0;
    (*denumenator) = 0;

    for (int i = 1; i < numLen; i++) {
        (*numenator) *= 10;
        (*numenator) += toDecimal(number[i]);
    }

    int mult2Count = numLen - 1, mult5Count = numLen - 1;

    while ((!((*numenator) % 2) && mult2Count > 0) || (!((*numenator) % 5)) && mult5Count > 0) {
        if (!((*numenator) % 2)) {
            (*numenator) /= 2;
            mult2Count--;
        }

        if (!((*numenator) % 5)) {
            (*numenator) /= 5;
            mult5Count--;
        }
    }

    (*denumenator) = pow(2, mult2Count) * pow(5, mult5Count);
}

void clear(char ***matrix, int size) {
    for (size -= 1; size >= 0; size--) {
        free((*matrix)[size]);
    }
    free(*matrix);
}

char *fToBase(char *number, int base) {
    int numLen = strlen(number), size = 16;

    char *res = (char *) malloc(sizeof(char) * (size)), *tmp = NULL;
    if (!res) {
        return NULL;
    }

    int actualSize = 0, iterations = 0;

    res[actualSize++] = '.';

    long numLong = 0;
    for (int i = 1; i < numLen; i++) {
        numLong *= 10;
        numLong += toDecimal(number[i]);
    }

    long partial = 0;
    while (numLong && iterations < 36) {
        iterations++;
        numLong *= base;
        partial = pow(10, numLen - 1);

        if (actualSize >= size - 1) {
            size *= 2;
            if (!(tmp = (char *) realloc(res, sizeof(char) * size))) {
                free(res);
                return NULL;
            }
            res = tmp;
        }

        res[actualSize++] = toAscii(numLong / partial);
        numLong %= partial;
    }

    if (!(tmp = (char *) realloc(res, sizeof(char) * (actualSize + 1)))) {
        free(res);
        return NULL;
    }

    res = tmp;
    res[actualSize] = '\0';
    return res;
}

int isFinalRepresentation(char ***result, int base, int count, ...) {
    if (base <= 1 || base > 36) {
        return 2;
    }
    if ((*result)) {
        return 2;
    } else {
        if (!((*result) = (char **) calloc(count, sizeof(char *)))) {
            return 1;
        }
    }

    va_list ptr;
    va_start(ptr, count);

    int numenator = 0, denumenator = 0, isEndless = 0, resultInd = 0;
    char *number = 0;

    for (int i = 0; i < count; i++) {
        number = va_arg(ptr, char*);
        isEndless = 0;
        inCommonFraction(&numenator, &denumenator, number);
        if (!(denumenator % 2) && (base % 2 != 0))
            isEndless++;
        if (!(denumenator % 5) && (base % 5 != 0))
            isEndless++;

        if (isEndless) {
            if (!((*result)[resultInd] = (char *) calloc(2, sizeof(char)))) {
                clear(result, resultInd);
                va_end(ptr);
                return 1;
            }
            strcpy((*result)[resultInd++], "0");
        } else {
            if (!((*result)[resultInd++] = fToBase(number, base))) {
                clear(result, resultInd);
                va_end(ptr);
                return 1;
            }
        }
    }
    va_end(ptr);
    return 0;
}

void printMatrix(char **matrix, int size) {
    for (int i = 0; i < size; i++) {
        printf("%s ", matrix[i]);
    }
    printf("\n\n");
}

int main() {
    int base = 0, count = 4;
    char *num1 = ".12345", *num2 = ".2569", *num3 = ".10001", *num4 = ".8";
    char **res = NULL;

    printf("Enter base: ");
    scanf("%d", &base);

    if (base < 1)
        return 2;

    int statusCode = isFinalRepresentation(&res, base, count, num1, num2, num3, num4);
    if (statusCode != 0)
        return statusCode;

    printf("\"0\" - number cant be displayed\n");
    printMatrix(res, count);

    clear(&res, count);
    return 0;
}