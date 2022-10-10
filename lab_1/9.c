#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>


int strLen(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int toDecimal(char c) {
    if (isdigit(c))
        return c - '0';
    return toupper(c) - 'A' + 10;
}

char toChar(int num) {
    if (num < 10)
        return num + '0';
    return num - 10 + 'A';
}

short isCorrect(int base, char *string) {
    int x = 0;
    while (string[x] != '\0') {
        if (!(isdigit(string[x]) || ('A' <= string[x] && string[x] <= 'Z')))
            return 0;
        if (base <= 10) {
            if (!('0' <= string[x] && string[x] <= toChar(base - 1)))
                return 0;
        } else if (!(isdigit(string[x]) ||
                     ('A' <= string[x] && string[x] <= toChar(base - 1)))) {
            return 0;
        }
        x++;
    }
    return 1;
}

void shift(char *string) {
    int x = 1;
    char tmp1 = string[0];
    char tmp2 = string[1];
    while (string[x - 1] != '\0') {
        string[x] = tmp1;
        tmp1 = tmp2;
        tmp2 = string[x + 1];
        x++;
    }
    string[0] = '0';
}

char *shiftLeft(char *num) {
    if (num[0] != '0') {
        return num;
    }
    int count = 0;
    for (int x = 0; x < strLen(num); x++) {
        if (num[x] == '0')
            count++;
        else
            break;
    }
    int numSize = strLen(num) - count;
    char *cNum = (char *) malloc(sizeof(char) * (numSize + 1));
    strcpy(cNum, num + count);
    return cNum;
}

short add(char *res, int base, char *num) {
    char *cNum = shiftLeft(num);
    if (cNum == NULL) {
        return 0;
    }
    int numInd = strLen(cNum) - 1;
    int resInd = strLen(res) - 1;
    int remember = 0;
    char addNum;
    while (resInd > 0 || numInd >= 0) {
        if (numInd < 0) {
            addNum = '0';
        } else {
            addNum = cNum[numInd];
        }
        int sum = toDecimal(addNum) + toDecimal(res[resInd])
                  + remember;
        res[resInd] = toChar(sum % base);
        remember = sum / base;
        resInd--;
        if (resInd < 0) {
            shift(res);
            resInd++;
        }
        numInd--;
    }
    if (remember) {
        res[resInd] = toChar(remember);
    }
    if (num != cNum)
        free(cNum);
    return 1;
}

char *sum(int *error, int base, int count, ...) {
    va_list args;
    va_start(args, count);
    int size = 2;
    char *res = (char *) malloc(sizeof(char) * size);
    res[0] = '0';
    res[1] = '\0';
    for (int x = 0; x < count; x++) {
        char *num = va_arg(args, char *);
        if (!isCorrect(base, num)) {
            free(res);
            *error = 2;
            return NULL;
        }
        if (strLen(num) + 1 > size - 1) {
            size = strLen(num) + 2;
            char *newRes = (char *) realloc(res, sizeof(char) * size);
            if (newRes == NULL) {
                free(res);
                *error = 1;
                return NULL;
            }
            res = newRes;
        }
        if (!add(res, base, num)) {
            free(res);
            *error = 1;
            return NULL;
        }
    }
    va_end(args);
    char *clearRes = shiftLeft(res);
    if (clearRes != res) {
        free(res);
    }
    return clearRes;
}

void errorMessage(int code) {
    switch (code) {
        case 1:
            printf("Insufficient memory\n");
            break;
        case 2:
            printf("Incorrect args\n");
            break;
        default:
            printf("???");
    }
}

int main() {
    int error = 0;
    char *res = sum(&error, 11, 2, "ZAE", "BALSA");
    if (error) {
        errorMessage(error);
        return 1;
    }
    printf("%s\n", res);
    return 0;
}