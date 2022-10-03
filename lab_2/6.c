#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


int strLen(const char *string) {
    int count = 0;
    while (string[count] != '\0') {
        count++;
    }
    return count;
}

short isInclude(char *string, char *substr) {
    int len, subLen, x;
    len = strLen(string);
    subLen = strLen(substr);
    if (len < 1 || subLen < 1 || subLen > len) {
        return 0;
    }
    x = 0;
    while (x < len) {
        if (string[x] == substr[0]) {
            for (int y = 1; y <= subLen; y++) {
                if (y == subLen) {
                    return 1;
                }
                if (string[x + y] != substr[y]) {
                    break;
                }
            }
        }
        x++;
    }
    return 0;
}

char **substrInPath(char *substr, int pathCount, ...) {
    int capacity = 5;
    int size = 0;
    char **result = (char **) malloc(sizeof(char*) * capacity);
    if (result == NULL) {
        return NULL;
    }
    va_list args;
    va_start(args, pathCount);
    char *path;
    for (int x = 0; x < pathCount; x++) {
        path = va_arg(args, char *);
        if (isInclude(path, substr)) {
            if (size == capacity) {
                char **newRes = (char **) realloc(result,
                                                  sizeof(char*) * capacity * capacity);
                if (newRes == NULL) {
                    free(result);
                    return NULL;
                }
                result = newRes;
                capacity *= capacity;
            }
            result[size] = path;
            size++;
        }
    }
    result[size] = NULL;
    return result;
}

int main(int argc, char *argv[]) {
    char path1[] = "/keee/kek";
    char path2[] = "yoyoy/KKK/lll";
    char path3[] = "kek";
    char **res = substrInPath(path3, 3, path1, path2, path3);
    if (res == NULL) {
        printf("Insufficient memory\n");
        return 1;
    }
    printf("'%s' contain paths:\n", path3);
    char **p = res;
    while (*p != NULL) {
        printf("%s\n", *p);
        p++;
    }
    free(res);
    return 0;
}