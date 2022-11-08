#include <stdio.h>
#include <stdlib.h>


int numBits(int k) {
    return 2 << (k - 1);
}

long long factorial(int n) {
    if (n > 22)
        return 2;
    else if (n == 0)
        return 1;
    else
        return (long long) n * factorial(n - 1);
}

int getMem(int l, int k) {
    long long a = factorial(k);
    long long b = factorial(l);
    long long c = factorial(k - l);
    return (int) (a / (b * c));
}

int checkBits(int l, int num, int straight) {
    int counter = 0;
    if (!straight) {
        while (num != 0) {
            counter += (num & 1);
            num >>= 1;
        }
    } else {
        while (num != 0) {
            if (!(num & 1)) {
                if (counter == l)
                    break;
                counter = 0;
            } else
                counter++;
            num >>= 1;
        }
    }
    return counter == l;
}

int bitsIn(int l, int k, int **list, int *len) {
    int lim = numBits(k);
    int size = getMem(l, k);
    *list = (int *) calloc(size, sizeof(int));
    if (*list == NULL) {
        return 1;
    }
    for (int i = 0; i < lim; i++) {
        if (checkBits(l, i, 0)) {
            (*list)[(*len)++] = i;
        }
    }
    return size ? 0 : -1;
}

int bitsForward(int l, int k, int **list, int *len) {
    int lim = numBits(k);
    int pred = numBits(l) - 1;
    int size = lim - pred;
    *list = (int *) malloc(size * sizeof(int));
    if (*list == NULL) {
        return 1;
    }
    for (int i = pred; i < lim; i++) {
        if (checkBits(l, i, 1)) {
            (*list)[(*len)++] = i;
        }
    }
    return size ? 0 : -1;
}

void print(int *list, int ln) {
    for (int i = 0; i < ln; i++) {
        printf("%d ", list[i]);
    }
    printf("\n");
}

int main() {
    int l, k;
    int statusCode = 0;
    int len = 0;
    int *list = NULL;
    int *straight = NULL;

    printf("Enter l: ");
    if (!scanf("%d", &l)) {
        return 7;
    }
    printf("\nEnter k: ");
    if (!scanf("%d", &k)) {
        return 7;
    }
    if (k > 31) {
        return 777; // overflow
    }
    if (l > k || l < 0 || k < 0) {
        return 7;
    }
    statusCode = bitsIn(l, k, &list, &len);
    if (statusCode != 0) {
        free(list);
        return statusCode;
    }
    print(list, len);
    free(list);
    printf("\n\n");
    len = 0;
    statusCode = bitsForward(l, k, &straight, &len);
    if (statusCode != 0) {
        free(straight);
        return statusCode;
    }
    print(straight, len);
    free(straight);
    return 0;
}