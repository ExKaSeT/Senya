#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>


typedef struct {
    char *defName;
    char *value;
} Item;

typedef struct {
    Item **array;
    int len;
    int capacity;
} List;

typedef struct {
    List **lists;
    int size;
} HashMap;

int listCreate(List **list) {
    *list = (List *) malloc(sizeof(List));
    if ((*list) == NULL) {
        return 1;
    }
    Item **array = (Item **) malloc(sizeof(Item *) * 5);
    if (array == NULL) {
        free((*list));
        return 1;
    }
    (*list)->array = array;
    (*list)->capacity = 5;
    (*list)->len = 0;
    return 0;
}

int listAdd(List *list, char *defName, char *value) {
    if (list == NULL) {
        return 2;
    }
    if (list->len == list->capacity) {
        list->capacity *= 2;
        Item **newArray = (Item **) realloc(list->array, sizeof(Item *) * list->capacity);
        if (newArray == NULL) {
            list->capacity /= 2;
            return 1;
        }
        list->array = newArray;
    }
    Item *newItem = (Item *) malloc(sizeof(Item));
    if (newItem == NULL)
        return 1;
    newItem->defName = defName;
    newItem->value = value;
    list->array[list->len] = newItem;
    list->len++;
    return 0;
}

void listDestroy(List *list) {
    if (list == NULL)
        return;
    if (list->len) {
        for (int x = 0; x < list->len; x++) {
            free(list->array[x]->defName);
            free(list->array[x]->value);
            free(list->array[x]);
        }
    }
    free(list->array);
    free(list);
}

int hashMapCreate(HashMap **hashMap, int size) { // 0 - set default size
    if (size != 0 && size < 3)
        return 2;
    if (size == 0)
        size = 3;
    *hashMap = (HashMap *) malloc(sizeof(HashMap));
    if ((*hashMap) == NULL) {
        return 1;
    }
    List **lists = (List **) malloc(sizeof(List *) * size);
    if (lists == NULL) {
        free((*hashMap));
        return 1;
    }
    List *list;
    for (int x = 0; x < size; x++) {
        int statusCode = listCreate(&list);
        if (statusCode != 0) {
            for (int y = 0; y < x; y++) {
                listDestroy(lists[y]);
            }
            free(lists);
            free((*hashMap));
            return statusCode;
        }
        lists[x] = list;
    }
    (*hashMap)->lists = lists;
    (*hashMap)->size = size;
    return 0;
}

void hashMapDestroy(HashMap *hashMap) {
    for (int x = 0; x < hashMap->size; x++) {
        listDestroy(hashMap->lists[x]);
    }
    free(hashMap->lists);
    free(hashMap);
}

void hashMapPrint(HashMap *hashMap) {
    for (int x = 0; x < hashMap->size; x++) {
        List *list = hashMap->lists[x];
        for (int y = 0; y < list->len; y++) {
            printf("%d| %s - %s\n", x, list->array[y]->defName, list->array[y]->value);
        }
    }
}

int toDecimal(char c) {
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('A' <= c && c <= 'Z')
        return c - 'A' + 10;
    else
        return c - 'a' + 36;
}

int hashMapGetHash(HashMap *hashMap, char *key) {
    int divisor = hashMap->size;
    unsigned long long num = 0;
    unsigned long long curPow = 1;
    for (int x = strlen(key) - 1; x >= 0; x--) {
        num = (num + toDecimal(key[x]) * curPow) % divisor;
        curPow = (curPow * 62) % divisor;
    }
    return num % divisor;
}

Item *hashMapGet(HashMap *hashMap, char *key) {
    if (hashMap == NULL || key == NULL)
        return NULL;
    List *list = hashMap->lists[hashMapGetHash(hashMap, key)];
    for (int x = 0; x < list->len; x++) {
        if (strcmp(key, list->array[x]->defName) == 0)
            return list->array[x];
    }
    return NULL;
}

int hashMapPut(HashMap *hashMap, char *key, char *value) {
    if (hashMapGet(hashMap, key) != NULL)
        return 2;
    int hash = hashMapGetHash(hashMap, key);
    int statusCode = listAdd(hashMap->lists[hash], key, value);
    if (statusCode != 0) {
        return statusCode;
    }
    return 0;
}

int hashMapRehashing(HashMap *hashMap) {
    HashMap *newMap;
    int newSize = hashMap->size + (hashMap->size > 50 ? hashMap->size / 5 : 11);
    int statusCode = hashMapCreate(&newMap, newSize);
    if (statusCode != 0)
        return statusCode;
    for (int x = 0; x < hashMap->size; x++) {
        List *list = hashMap->lists[x];
        if (list->len != 0) {
            for (int y = 0; y < list->len; y++) {
                statusCode = hashMapPut(newMap, list->array[y]->defName, list->array[y]->value);
                if (statusCode != 0) {
                    hashMapDestroy(newMap);
                    return statusCode;
                }
            }
        }
    }
    List **listsTmp = hashMap->lists;
    int sizeTmp = hashMap->size;
    hashMap->lists = newMap->lists;
    hashMap->size = newMap->size;
    newMap->lists = listsTmp;
    newMap->size = sizeTmp;
    hashMapDestroy(newMap);
    return 0;
}

int hashMapCheckAndRehash(HashMap *hashMap) {
    int minLen = INT_MAX;
    int maxLen = 0;
    for (int x = 0; x < hashMap->size; x++) {
        int len = hashMap->lists[x]->len;
        if (len == 0)
            continue;
        if (len > maxLen)
            maxLen = len;
        if (len < minLen)
            minLen = len;
    }
    int statusCode;
    if ((double) maxLen / minLen > 2) {
        statusCode = hashMapRehashing(hashMap);
        if (statusCode != 0)
            return statusCode;
        statusCode = hashMapCheckAndRehash(hashMap);
        if (statusCode != 0)
            return statusCode;
    }
    return 0;
}

int nextLine(FILE *file) {
    char c;
    int isEOF = 0;
    while (1) {
        c = fgetc(file);
        if (c == EOF) {
            isEOF = 1;
            break;
        } else if (c == '\n') {
            break;
        }
    }
    if (isEOF)
        return -1;
    return 0;
}

int readString(FILE *file, char **result, const char *separators, char *metSeparator) {
    int isEOF = 0;
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
        int isBreak = 0;
        while (separators[x] != '\0') {
            if (c == separators[x]) {
                if (metSeparator != NULL)
                    *metSeparator = c;
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
        res[count] = c;
        count++;
    }
    if (count == 0) {
        free(res);
        if (isEOF)
            return -2;
        return 2;
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

int readDirectives(HashMap **result, FILE *data) {
    HashMap *map;
    int statusCode = hashMapCreate(&map, 0);
    if (statusCode != 0)
        return statusCode;
    char *string, *defName, *value;
    char ch;
    while ((ch = getc(data)) == '#') {
        statusCode = readString(data, &string, " ", NULL);
        if (statusCode != 0) {
            hashMapDestroy(map);
            return statusCode;
        }
        free(string);
        statusCode = readString(data, &defName, " ", NULL);
        if (statusCode != 0) {
            hashMapDestroy(map);
            return statusCode;
        }
        statusCode = readString(data, &value, "\n", NULL);
        if (statusCode != 0) {
            hashMapDestroy(map);
            free(defName);
            return statusCode;
        }
        statusCode = hashMapPut(map, defName, value);
        if (statusCode != 0) {
            hashMapDestroy(map);
            free(defName);
            free(value);
            return statusCode;
        }
    }
    ungetc(ch, data);
    statusCode = hashMapCheckAndRehash(map);
    if (statusCode != 0) {
        hashMapDestroy(map);
        return statusCode;
    }
    *result = map;
    return 0;
}

int replaceDirectives(HashMap *directives, FILE *data) {
    if (directives == NULL || data == NULL)
        return 2;
    FILE *out;
    if ((out = fopen("out.txt", "w")) == NULL) {
        return 10;
    }
    char *string;
    char metSeparator = 0;
    int statusCode = 0;
    while (statusCode != -1) {
        statusCode = readString(data, &string, " \n\t", &metSeparator);
        if (statusCode == 1) {
            fclose(out);
            return statusCode;
        } else if (statusCode == 2) { // empty string
            putc(metSeparator, out);
            continue;
        } else if (statusCode == -2) { // EOF && empty string
            break;
        }
        int strLen = strlen(string);
        char *startPtr = string;
        char *remainsAdd = NULL; // NULL - full string
        for (int start = 0; start < strLen; start++) { // inclusive
            for (int end = start + 1; end <= strLen; end++) { // exclusive
                char tmp = string[end];
                string[end] = '\0';
//                printf("%s| %d %d\n", startPtr, start, end);
                Item *item = hashMapGet(directives, startPtr);
                if (item != NULL) {
                    for (int x = 0; x < start; x++) {
                        putc(string[x], out);
                    }
                    fprintf(out, "%s", item->value);
                    startPtr = string + end;
                    start = end;
                    remainsAdd = string + end;
                }
                string[end] = tmp;
            }
            startPtr++;
        }
        if (remainsAdd == NULL) {
            fprintf(out, "%s", string);
        } else if (*remainsAdd != '\n') {
            fprintf(out, "%s", remainsAdd);
        }
        putc(metSeparator, out);
        free(string);
        metSeparator = 0;
    }
    fclose(out);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        return 69;

    FILE *data;
    if ((data = fopen(argv[1], "r")) == NULL) {
        return 10;
    }

    HashMap *map;
    int statusCode = readDirectives(&map, data);
    if (statusCode != 0) {
        fclose(data);
        return statusCode;
    }
    statusCode = replaceDirectives(map, data);
    if (statusCode != 0) {
        fclose(data);
        hashMapDestroy(map);
        return statusCode;
    }

    hashMapDestroy(map);
    fclose(data);
    return 0;
}