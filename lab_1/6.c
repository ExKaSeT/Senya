#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


int separator(int sep) {
    if (sep == ' ' || sep == '\n' || sep == '\t') {
        return 1;
    }
    return 0;
}

char *strInit(int size) {
    char *buf = (char *) malloc(sizeof(char) * size);
    memset(buf, 0, size);
    return buf;
}

char *strRealloc(char *from, int currSize) {
    char *res = strInit(currSize);
    strcpy(res, from);
    free(from);
    return res;
}

int reversedToI(char *s, int base, int size) {
    int n = 0, index = size;
    int tmp = 1;
    while (index-- != 0) {
        n += (isdigit(s[index]) ? (s[index] - '0') : (tolower(s[index]) - 'a' + 10)) * tmp;
        tmp *= base;
    }
    return n;
}

int main(int argc, char *argv[]) {
    FILE *in = NULL;
    FILE *out = NULL;
    if (!(in = fopen(argv[1], "r")) || !(out = fopen("out.txt", "w"))) {
        printf("File cant be opened\n");
        return 69;
    }
    int base = 2, c, _c = 0, sized = 0, sizedCurr = 16, num;
    char *buff = strInit(sizedCurr);
    while ((c = fgetc(in)) != EOF) {
        if (isalnum(c)) {
            if (sized == sizedCurr - 2) {
                buff[sized] = '\0';
                buff = strRealloc(buff, sizedCurr *= 2);
            }
            if (isdigit(c)) {
                if (base < c - '0') {
                    base = c - '0' + 1;
                }
            } else {
                if (base < tolower(c) - 'a' + 10) {
                    base = tolower(c) - 'a' + 11;
                }
            }
            buff[sized++] = c;
        } else if (isalnum(_c) && separator(c)) {
            if (sized == sizedCurr - 2) {
                buff[sized] = '\0';
                buff = strRealloc(buff, sizedCurr + 1);
            }
            buff[sized] = '\0';
            num = reversedToI(buff, base, sized);
            fprintf(out, "%s %d %d\n", buff, base, num);
            free(buff);
            base = 2;
            sizedCurr = 16;
            sized = 0;
            buff = strInit(sizedCurr);
        }
        _c = c;
    }
    return 0;
}