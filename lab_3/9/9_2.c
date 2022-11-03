#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "str.h"


typedef struct {
    String *string;
    int priority;
} T;

typedef struct {
    int size;
    int capacity;
    T **data;
} Heap;

int heapCreate(Heap **get) {
    Heap *heap = (Heap *) malloc(sizeof(Heap));
    if (heap == NULL) {
        return 1;
    }
    T **data = (T **) malloc(sizeof(T *) * 5);
    if (data == NULL) {
        free(heap);
        return 1;
    }
    heap->size = 0;
    heap->capacity = 5;
    heap->data = data;
    *get = heap;
    return 0;
}

void freeHeap(Heap *heap) {
    if (heap->size == 0) {
        free(heap);
        return;
    }
    T **data = heap->data;
    for (int x = 0; x < heap->size; x++) {
        freeString(data[x]->string);
        free(data[x]);
    }
    free(data);
    free(heap);
}

int queueInsert(Heap *heap, T *data) {
    if (heap->size == heap->capacity) {
        heap->capacity *= 2;
        T **newData = (T **) realloc(heap->data, sizeof(T *) * heap->capacity);
        if (newData == NULL) {
            heap->capacity /= 2;
            return 1;
        }
        heap->data = newData;
    }
    int i, parent;
    i = heap->size;
    T **h = heap->data;
    h[i] = data;
    parent = (i - 1) / 2;
    while (parent >= 0 && i > 0) {
        if (h[i]->priority > h[parent]->priority) {
            T *temp = h[i];
            h[i] = h[parent];
            h[parent] = temp;
        }
        i = parent;
        parent = (i - 1) / 2;
    }
    heap->size++;
    return 0;
}

T *queueGetValue(Heap *heap) {
    return heap->data[0];
}

void heapify(Heap *heap, int index) {
    int left, right;
    T *temp;
    T **h = heap->data;
    left = 2 * index + 1;
    right = 2 * index + 2;
    if (right < heap->size) {
        if (h[index]->priority < h[right]->priority) {
            temp = h[index];
            h[index] = h[right];
            h[right] = temp;
            heapify(heap, right);
        }
    }
    if (left < heap->size) {
        if (h[index]->priority < h[left]->priority) {
            temp = h[index];
            h[index] = h[left];
            h[left] = temp;
            heapify(heap, left);
        }
    }
}

T *queueExtractValue(Heap *heap) {
    if (heap->size == 0)
        return NULL;
    T *data;
    data = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heapify(heap, 0);
    heap->size--;
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

void print(Heap *heap) {
    printf("-----------------------------\n");
    int i = 0;
    int k = 1;
    while (i < heap->size) {
        while ((i < k) && (i < heap->size)) {
            printf("%d ", heap->data[i]->priority);
            printString(heap->data[i]->string);
            printf("        ");
            i++;
        }
        printf("\n");
        k = k * 2 + 1;
    }
    printf("-----------------------------\n");
}

int readData(char **filenames, int count, Heap *heap) {
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
            statusCode = queueInsert(heap, data);
            if (statusCode != 0) {
                for (int y = 0; y < count; y++) {
                    fclose(files[y]);
                }
                free(files);
                free(data);
                freeString(str);
                return 5;
            }
            print(heap);
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

int main(int argc, char *argv[]) {
    Heap *heap;
    int statusCode = heapCreate(&heap);
    if (statusCode != 0)
        return statusCode;

    statusCode = readData(argv + 1, argc - 1, heap);
    if (statusCode != 0) {
        freeHeap(heap);
        return statusCode;
    }

    print(heap);

    FILE *out = fopen("out.txt", "w");
    if (out == NULL) {
        freeHeap(heap);
        return 777;
    }

    T *data;
    while ((data = queueExtractValue(heap)) != NULL) {
        char *string = (char *) malloc(sizeof(char) * (data->string->len + 1));
        if (string == NULL) {
            freeHeap(heap);
            fclose(out);
            freeString(data->string);
            free(data);
            return 777;
        }
        string[data->string->len] = '\0';
        memcpy(string, data->string->data, sizeof(char) * data->string->len);
        if (fprintf(out, "prior=%d task='%s'\n", data->priority, string) < 0) {
            freeHeap(heap);
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

    freeHeap(heap);
    return 0;
}
