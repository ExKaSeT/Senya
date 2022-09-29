#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main() {
    int size = 10;
    int list[size];
    for (int i = 0; i < size; i++) {
        list[i] = rand() % 10000;
        printf("%d ", list[i]);
    }
    printf("\n");

    int min = INT_MAX;
    int max = INT_MIN;
    int indMin;
    int indMax;
    for (int i = 0; i < size; i++) {
        if (list[i] > max) {
            max = list[i];
            indMax = i;
        }
        if (list[i] < min) {
            min = list[i];
            indMin = i;
        }
    }

    int temp = list[indMax];
    list[indMax] = list[indMin];
    list[indMin] = temp;

    for (int i = 0; i < size; i++) {
        printf("%d ", list[i]);
    }
    printf("\n");
    return 0;
}