#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>


int len(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int toDecimal(int c) {
    c = toupper(c);
    char arr[36] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < 36; i++) {
        if (arr[i] == c)
            return i;
    }
    return -1;
}

void shiftLeft(char *str, const int *length) {
    for (int i = 0; i < *length; i++) {
        *str = *(str + 1);
        str++;
    }
    *str = '\0';
}

void shift(char *str) {
    int length = len(str);
    char temp1 = *str;
    char temp2;
    *str = '0';
    str++;
    for (int i = 0; i < length; i++) {
        temp2 = *str;
        *str = temp1;
        temp1 = temp2;
        str++;
    }
    *str = '\0';
}

short add(int base, char *res, char *num) {
    char arr[36] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int lengthNum = len(num);
    int lengthRes = len(res);
    while (lengthRes <= lengthNum) {
        shift(res);
        lengthRes = len(res);
    }
    while (*res != '\0')
        res++;
    while (*num != '\0')
        num++;
    num--;
    res--;
    int remember = 0;
    for (int i = 0; i < lengthNum; i++) {
        if (toDecimal(*num) >= base) {
            return 0;
        }
        int sum = toDecimal(*num) + toDecimal(*res) + remember;
        if (sum >= base)
            remember = 1;
        else
            remember = 0;
        *res = arr[sum % base];
        res--;
        num--;
    }
    while (remember) {
        int sum = toDecimal(*res) + remember;
        if (sum >= base)
            remember = 1;
        else
            remember = 0;
        *res = arr[sum % base];
        res--;
    }
    res++;
    return 1;
}

short sum(char *res, int base, int argc, ...) {
    va_list args;
    va_start(args, argc);
    res[0] = '0';
    res[1] = '\0';
    for (int i = 0; i < argc; i++) {
        if (!add(base, res, va_arg(args, char *))) {
            return 0;
        }
    }
    int length = len(res);
    while (res[0] == '0')
        shiftLeft(res, &length);
    va_end(args);
    return 1;
}

int main() {
    char res[100];
    if (!sum(res, 11, 2, "30A", "10")) {
        printf("Incorrect args\n");
        return 1;
    }
    printf("%s\n", res);
    return 0;
}