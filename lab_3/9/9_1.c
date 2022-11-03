#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "str.h"


typedef struct {
    String *string;
    int priority;
} T;

typedef struct Node Node;
struct Node {
    T *data;
    Node *next;
    Node *prev;
};

typedef struct {
    int size;
    Node *first;
    Node *last;
} List;

int listCreate(List **get) {
    List *list = (List *) malloc(sizeof(List));
    if (list == NULL) {
        return 1;
    }
    list->size = 0;
    list->first = NULL;
    list->last = NULL;
    *get = list;
    return 0;
}

void freeList(List *list) {
    if (list->size == 0) {
        free(list);
        return;
    }
    Node *node = list->first->next;
    for (int x = 2; x < list->size; x++) {
        freeString(node->prev->data->string);
        free(node->prev->data);
        free(node->prev);
        node = node->next;
    }
    freeString(node->data->string);
    free(node->data);
    free(node);
    free(list);
}

int queueInsert(List *list, T *data) {
    Node *newNode = (Node *) malloc(sizeof(Node));
    if (newNode == NULL) {
        return 1;
    }
    newNode->data = data;
    newNode->next = NULL;
    newNode->prev = NULL;
    if (list->size == 0) { // 0 элементов
        list->first = newNode;
        list->last = newNode;
        list->size++;
        return 0;
    }
    int index = 0;
    Node *node = list->first;
    while (node != NULL && data->priority > node->data->priority) {
        node = node->next;
        index++;
    }
    if (index == 0) { // вставка в начало
        newNode->next = list->first;
        list->first->prev = newNode;
        list->first = newNode;
    } else if (index == list->size) { // вставка в конец
        newNode->prev = list->last;
        list->last->next = newNode;
        list->last = newNode;
    } else {
        Node *prevNode = node->prev;
        Node *nextNode = node;
        prevNode->next = newNode;
        nextNode->prev = newNode;
        newNode->next = nextNode;
        newNode->prev = prevNode;
    }
    list->size++;
    return 0;
}

T *queueGetValue(List *list) {
    return list->last->data;
}

T *queueExtractValue(List *list) {
    if (list->size == 0)
        return NULL;
    T *data = list->last->data;
    if (list->size == 1) {
        free(list->last);
        list->last = NULL;
    } else {
        list->last = list->last->prev;
        free(list->last->next);
        list->last->next = NULL;
    }
    list->size--;
    return data;
}

short isNum(char c) {
    return ('0' <= c && c <= '9');
}

short readString(FILE *file, char **result, char *separators, short (*isCorrectChar)(char)) {
    short isEOF = 0;
    int resCapacity = 1;
    char *res = (char *) malloc(sizeof(char) * resCapacity);
    if (res == NULL)
        return 1;
    char c;
    int count = 0;
    while (1) {
        c = fgetc(file);
        if (c == EOF) {
            isEOF = 1;
            break;
        }
        int x = 0;
        short isBreak = 0;
        while (separators[x] != '\0') {
            if (c == separators[x]) {
                isBreak = 1;
                break;
            }
            x++;
        }
        if (isBreak)
            break;
        if (count == resCapacity) {
            resCapacity *= 2;
            char *newRes = (char *) realloc(res, sizeof(char) * resCapacity);
            if (newRes == NULL) {
                free(res);
                return 1;
            }
            res = newRes;
        }
        if (isCorrectChar != NULL && !isCorrectChar(c)) {
            free(res);
            return 2;
        }
        res[count] = c;
        count++;
    }
    if (count == 0) {
        free(res);
        return 3;
    }
    char *newRes = (char *) realloc(res, sizeof(char) * (count + 1));
    if (newRes == NULL) {
        free(res);
        return 1;
    }
    res = newRes;
    res[count] = '\0';
    *result = res;
    if (isEOF)
        return -1;
    return 0;
}

int parseInt(const char *string) { // correct number in str
    int x = 0;
    while (string[x] == '0')
        x++;
    if (string[x] == '\0')
        return 0;
    int num = (string[x] - '0');
    x++;
    while (string[x] != '\0') {
        num = num * 10 + (string[x] - '0');
        x++;
    }
    return num;
}

int readData(char **filenames, int count, List *list) {
    FILE **files = (FILE **) malloc(sizeof(FILE *) * count);
    if (files == NULL) {
        return 1;
    }
    for (int x = 0; x < count; x++) {
        FILE *file = fopen(filenames[x], "r");
        if (file == NULL) {
            for (int y = 0; y < x; y++) {
                fclose(files[y]);
            }
            free(files);
            return 2;
        }
        files[x] = file;
    }
    while (1) {
        int isBreak = 1;
        T *data;
        for (int x = 0; x < count; x++) {
            if (files[x] == NULL)
                continue;
            isBreak = 0;
            char *string;
            int statusCode;
            statusCode = readString(files[x], &string, "=", NULL);
            if (statusCode == -1) {
                files[x] = NULL;
                continue;
            }
            if (statusCode != 0) {
                for (int y = 0; y < count; y++) {
                    fclose(files[y]);
                }
                free(files);
                return statusCode;
            }
            free(string);
            statusCode = readString(files[x], &string, " ", isNum);
            if (statusCode != 0) {
                for (int y = 0; y < count; y++) {
                    fclose(files[y]);
                }
                free(files);
                return statusCode;
            }
            int priority = parseInt(string);
            free(string);
            if (priority < 0 || priority > 10) {
                for (int y = 0; y < count; y++) {
                    fclose(files[y]);
                }
                free(files);
                return 7;
            }
            statusCode = readString(files[x], &string, "'", NULL);
            if (statusCode != 0) {
                for (int y = 0; y < count; y++) {
                    fclose(files[y]);
                }
                free(files);
                return statusCode;
            }
            free(string);
            statusCode = readString(files[x], &string, "'", NULL);
            if (statusCode != 0) {
                for (int y = 0; y < count; y++) {
                    fclose(files[y]);
                }
                free(files);
                return statusCode;
            }
            String *str;
            statusCode = createString(&str, string);
            free(string);
            if (statusCode != 0) {
                for (int y = 0; y < count; y++) {
                    fclose(files[y]);
                }
                free(files);
                return statusCode;
            }
            char lastCharInLine = getc(files[x]);
            if (lastCharInLine != '\n' && lastCharInLine != EOF) {
                for (int y = 0; y < count; y++) {
                    fclose(files[y]);
                }
                free(files);
                freeString(str);
                return 4;
            }
            if (lastCharInLine == EOF) {
                files[x] = NULL;
            }
            data = (T *) malloc(sizeof(T));
            if (data == NULL) {
                for (int y = 0; y < count; y++) {
                    fclose(files[y]);
                }
                free(files);
                freeString(str);
                return 5;
            }
            data->priority = priority;
            data->string = str;
            statusCode = queueInsert(list, data);
            if (statusCode != 0) {
                for (int y = 0; y < count; y++) {
                    fclose(files[y]);
                }
                free(files);
                free(data);
                freeString(str);
                return 5;
            }
        }
        if (isBreak)
            break;
    }
    for (int y = 0; y < count; y++) {
        fclose(files[y]);
    }
    free(files);
    return 0;
}

void print(List *list) {
    Node *node = list->first;
    printf("-----------------\n");
    for (int x = 0; x < list->size; x++) {
        printf("%d ", node->data->priority);
        printString(node->data->string);
        printf("\n");
        node = node->next;
    }
    printf("-----------------\n");
}

int main(int argc, char *argv[]) {
    List *list;
    int statusCode = listCreate(&list);
    if (statusCode != 0)
        return statusCode;

    statusCode = readData(argv + 1, argc - 1, list);
    if (statusCode != 0) {
        freeList(list);
        return statusCode;
    }

    print(list);

    FILE *out = fopen("out.txt", "w");
    if (out == NULL) {
        freeList(list);
        return 777;
    }

    T *data;
    while ((data = queueExtractValue(list)) != NULL) {
        char *string = (char *) malloc(sizeof(char) * (data->string->len + 1));
        if (string == NULL) {
            freeList(list);
            fclose(out);
            freeString(data->string);
            free(data);
            return 777;
        }
        string[data->string->len] = '\0';
        memcpy(string, data->string->data, sizeof(char) * data->string->len);
        if (fprintf(out, "prior=%d task='%s'\n", data->priority, string) < 0) {
            freeList(list);
            fclose(out);
            freeString(data->string);
            free(data);
            return 3;
        }
        freeString(data->string);
        free(string);
        free(data);
    }

    fclose(out);
    freeList(list);
    return 0;
}
