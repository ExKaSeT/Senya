#include <stdlib.h>
#include <stdio.h>


void printArray(int *array, int size) {
    printf("[ ");
    for (int x = 0; x < size; x++) {
        printf("%d, ", array[x]);
    }
    printf("\b\b ]\n");
}

void swapElem(int *el1, int *el2) {
    int temp = *el1;
    *el1 = *el2;
    *el2 = temp;
}

int justUniq(int *array, int len) {
    int dicLen = 0;
    int key[len];
    int value[len];
    short isFind = 0;
    for (int x = 0; x < len; x++) {
        for (int y = 0; y < dicLen; y++) {
            if (array[x] == key[y]) {
                value[y]++;
                isFind = 1;
                break;
            }
        }
        if (!isFind) {
            key[dicLen] = array[x];
            value[dicLen] = 1;
            dicLen++;
        }
        isFind = 0;
    }
    int index = 0;
    for (int x = 0; x < dicLen; x++) {
        if (value[x] == 1) {
            array[index] = key[x];
            index++;
        }
    }
    return index;
}

int main(int argc, char *argv[]) {
    int length = 10;
    int array[length];
    int beginRange;
    int endRange;
    printf("Enter begin and end of range separated by space\n");
    scanf("%d %d", &beginRange, &endRange);
    if (beginRange > endRange) {
        printf("Incorrect range\n");
        return 1;
    }
    for (int x = 0; x < length; x++) {
        array[x] =  (rand() % (endRange - beginRange)) + beginRange;
    }
    printArray(array, length);
    printf("1 - swap min max\n2 - find unique\n");
    short cmd;
    scanf("%d", &cmd);
    if (cmd == 1) {
        int *max = array;
        int *min = array;
        for (int x = 1; x < length; x++) {
            if (array[x] < *min) {
                min = array + x;
            } else if (array[x] > *max) {
                max = array + x;
            }
        }
        swapElem(min, max);
        printArray(array, length);
        return 0;
    } else if (cmd == 2) {
        length = justUniq(array, length);
        printArray(array, length);
        return 0;
    } else {
        printf("Incorrect command\n");
        return 1;
    }
}