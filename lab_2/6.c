#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

typedef char* ListType;

typedef struct {
    ListType *data;
    int size;
    int capacity;
} List;

short add(List *list, ListType data) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        ListType *newData = (ListType *) realloc(list->data, sizeof(ListType) * (list->capacity));
        if (newData == NULL) {
            list->capacity /= 2;
            return 0;
        }
        list->data = newData;
    }
    list->data[list->size] = data;
    list->size++;
    return 1;
}

void destroy(List *list) {
    free(list->data);
    free(list);
}

List *createList() {
    List *list = (List *) malloc(sizeof(List));
    ListType *data = (ListType *) malloc(sizeof(int) * 5);
    if (list == NULL || data == NULL) {
        return NULL;
    }
    list->size = 0;
    list->data = data;
    list->capacity = 5;
    return list;
}

int strLen(const char *string) {
    int count = 0;
    while (string[count] != '\0') {
        count++;
    }
    return count;
}

int KMP(char *pat, char *txt) {
    int M = strLen(pat);
    int N = strLen(txt);
    int lps[M];
    int len = 0;
    lps[0] = 0;
    int i = 1;
    while (i < M) {
        if (pat[i] == pat[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
    i = 0;
    int j = 0;
    while ((N - i) >= (M - j)) {
        if (pat[j] == txt[i]) {
            j++;
            i++;
        }
        if (j == M) {
            return i - j;
        } else if (i < N && pat[j] != txt[i]) {
            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
    }
    return -1;
}

short isInclude(char *string, char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return -1;
    }
    int buffLen = strLen(string) > 50 ? strLen(string) * 3 : 100;
    char *buff = (char *) malloc(sizeof(char) * buffLen);
    if (buff == NULL) {
        return -1;
    }
    int result;
    char c;
    int ind = 0;
    while ((c = fgetc(file)) != EOF) {
        buff[ind] = c;
        ind++;
        if (ind + 1 == buffLen) {
            buff[ind] = '\0';
            result = KMP(string, buff);
            if (result >= 0) {
                free(buff);
                return 1;
            }
            ind = 0;
        }
    }
//    printf("%s\n\n", buff);
    buff[ind] = '\0';
    result = KMP(string, buff);
    if (result >= 0) {
        free(buff);
        return 1;
    }
    free(buff);
    return 0;
}

List *strInFiles(char *string, int pathCount, ...) {
    List *result = createList();
    if (result == NULL) {
        return NULL;
    }
    va_list args;
    va_start(args, pathCount);
    char *path;
    for (int x = 0; x < pathCount; x++) {
        path = va_arg(args, char *);
        short dich = isInclude(string, path);
        if (dich == -1) {
            destroy(result);
            return NULL;
        } else if (dich == 1){
            if (!add(result, path)) {
                destroy(result);
                return NULL;
            }
        }
    }
    return result;
}

int main(int argc, char *argv[]) {
    char path1[] = "moi";
    char path2[] = "homyak";
    char string[] = ":/";
    List *result = strInFiles(string, 2, path1, path2);
    if (result == NULL) {
        printf("Insufficient memory\n");
        return 1;
    }
    printf("'%s' contain files:\n", string);
    for (int x = 0; x < result->size; x++) {
        printf("%s\n", result->data[x]);
    }
    destroy(result);
    return 0;
}