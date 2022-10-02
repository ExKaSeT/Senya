#include <stdlib.h>
#include <stdio.h>


void fillArray(int *array, int len) {
    for (int x = 0; x < len; x++) {
        array[x] = rand() % 10;
    }
}

int main(int argc, char* argv[]) {
    int lenA, lenB;
    lenA = rand() % 10;
    lenB = rand() % 10;
    int A[lenA], B[lenB], C[lenA];
    fillArray(A, lenA);
    fillArray(B, lenB);
    for (int x = 0; x < lenA; x++) {
        if (x < lenB) {
            C[x] = A[x] + B[x];
        } else {
            C[x] = A[x] + B[lenB - 1];
        }
    }
    return 0;
}