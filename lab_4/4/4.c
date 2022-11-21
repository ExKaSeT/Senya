#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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

typedef struct {
    char name;
    int *array;
    int len;
    int capacity;
} List;

int listCreate(List **list, char name) {
    *list = (List *) malloc(sizeof(List));
    if ((*list) == NULL) {
        return 1;
    }
    int *array = (int *) malloc(sizeof(int) * 5);
    if (array == NULL) {
        free((*list));
        return 1;
    }
    (*list)->name = name;
    (*list)->array = array;
    (*list)->capacity = 5;
    (*list)->len = 0;
    return 0;
}

void listPrint(List *list) {
    printf("[ %c ] ", list->name);
    for (int x = 0; x < list->len; x++) {
        printf("%d ", list->array[x]);
    }
    printf("\n");
}

int listAdd(List *list, int value) {
    if (list == NULL) {
        return 2;
    }
    if (list->len == list->capacity) {
        list->capacity *= 2;
        int *newArray = (int *) realloc(list->array, sizeof(int) * list->capacity);
        if (newArray == NULL) {
            list->capacity /= 2;
            return 1;
        }
        list->array = newArray;
    }
    list->array[list->len] = value;
    list->len++;
    return 0;
}

void listDestroy(List *list) {
    if (list == NULL)
        return;
    free(list->array);
    free(list);
}

int listConcat(List *dest, List *source) {
    if (dest == NULL || source == NULL)
        return 2;
    if (source->len == 0)
        return 0;
    int statusCode = 0;
    for (int x = 0; x < source->len; x++) {
        statusCode = listAdd(dest, source->array[x]);
        if (statusCode != 0) {
            listDestroy(dest);
            return statusCode;
        }
    }
    return 0;
}

int sortByInc(int *a, int *b) {
    return *a - *b;
}

int sortByDec(int *a, int *b) {
    return *b - *a;
}

int listSort(List *list, int order) {
    if (list == NULL)
        return 2;
    if (order) {
        qsort(list->array, list->len, sizeof(int), (int (*)(const void *, const void *)) sortByInc);
    } else {
        qsort(list->array, list->len, sizeof(int), (int (*)(const void *, const void *)) sortByDec);
    }
    return 0;
}

List *listsSearch(List **list, int len, char name) {
    for (int x = 0; x < len; x++) {
        if (list[x]->name == name) {
            return list[x];
        }
    }
    return NULL;
}

int listsSearchIndex(List **list, int len, char name) {
    for (int x = 0; x < len; x++) {
        if (list[x]->name == name) {
            return x;
        }
    }
    return -1;
}

int listsAdd(List ***lists, int *len, List *list) {
    List **newList;
    if (*len == 0) {
        newList = (List **) malloc(sizeof(List *));
    } else {
        newList = (List **) realloc(*lists, sizeof(List *) * (*len + 1));
    }
    if (newList == NULL) {
        return 1;
    }
    (*lists) = newList;
    (*lists)[*len] = list;
    (*len)++;
    return 0;
}

void listsDestroy(List **lists, int len) {
    for (int x = 0; x < len; x++) {
        listDestroy(lists[x]);
    }
    free(lists);
}

int parseInt(const char *string) { // correct number in str
    int isNegative = 0;
    int x = 0;
    if (string[0] == '-') {
        isNegative = 1;
        x++;
    }
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
    if (isNegative)
        num *= -1;
    return num;
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

int parseInts(int *result, int resLen, const char *string) {
    if (resLen < 1)
        return 2;
    int count = 0;
    char tmp[11];
    int tmpLen = 0;
    int x = 0;
    int isNegative = 0;
    do {
        if ('0' <= string[x] && string[x] <= '9') {
            if (tmpLen == 9)
                return 2;
            tmp[tmpLen] = string[x];
            tmpLen++;
        } else if (string[x] == ',' || string[x] == '\0') {
            tmp[tmpLen] = '\0';
            result[count] = parseInt(tmp);
            if (isNegative)
                result[count] *= -1;
            isNegative = 0;
            count++;
            tmpLen = 0;
            if (count == resLen)
                break;
        } else if (string[x] == '-') {
            isNegative = 1;
        } else {
            isNegative = 0;
        }
        x++;
    } while (string[x - 1] != '\0');
    return 0;
}

int funcLoad(List ***lists, int *len, char *string) { // string: "A, in.txt"
    if (strlen(string) > 50)
        return 2;
    int statusCode = 0;
    List *list;
    statusCode = listCreate(&list, string[0]);
    if (statusCode != 0)
        return statusCode;
    char filename[50];
    strcpy(filename, string + 3);
    FILE *data;
    if ((data = fopen(filename, "r")) == NULL) {
        listDestroy(list);
        return 10;
    }
    char *num;
    while (statusCode != -1) {
        statusCode = readString(data, &num, " \n\t", NULL);
        if (statusCode == 2) {
            continue;
        } else if (statusCode == 1) {
            fclose(data);
            listDestroy(list);
            return statusCode;
        }
        int x = 0;
        if (num[x] == '-')
            x++;
        while (num[x] != '\0') {
            if (num[x] < '0' || num[x] > '9') {
                free(num);
                fclose(data);
                listDestroy(list);
                return 2;
            }
            x++;
        }
        int statusCode1 = listAdd(list, parseInt(num));
        free(num);
        if (statusCode1 != 0) {
            fclose(data);
            listDestroy(list);
            return statusCode1;
        }
    }
    fclose(data);
    List *existList = listsSearch(*lists, *len, string[0]);
    if (existList == NULL) {
        statusCode = listsAdd(lists, len, list);
        if (statusCode != 0) {
            listDestroy(list);
            return statusCode;
        }
    } else {
        statusCode = listConcat(existList, list);
        listDestroy(list);
        if (statusCode != 0) {
            return statusCode;
        }
    }
    return 0;
}

int funcSave(List **lists, int len, char *string) { // string: "A, out.txt"
    if (strlen(string) > 50)
        return 2;
    int statusCode = 0;
    List *list = listsSearch(lists, len, string[0]);
    if (list == NULL)
        return 3;
    char filename[50];
    strcpy(filename, string + 3);
    FILE *data;
    if ((data = fopen(filename, "w")) == NULL) {
        return 10;
    }
    for (int x = 0; x < list->len; x++) {
        fprintf(data, "%d ", list->array[x]);
    }
    fclose(data);
    return 0;
}

int funcRand(List ***lists, int *len, char *string) { // string: "A, 10, 2, 6"
    int count, a, b;
    int i[3];
    int statusCode = parseInts(i, 3, string + 3);
    if (statusCode != 0) {
        return statusCode;
    }
    count = i[0];
    a = i[1];
    b = i[2];
    if (count < 1 || a > b)
        return 2;
    List *list;
    statusCode = listCreate(&list, string[0]);
    if (statusCode != 0)
        return statusCode;
    for (int x = 0; x < count; x++) {
        statusCode = listAdd(list, a + rand() % (b - a + 1));
        if (statusCode != 0) {
            listDestroy(list);
            return statusCode;
        }
    }
    List *existList = listsSearch(*lists, *len, string[0]);
    if (existList == NULL) {
        statusCode = listsAdd(lists, len, list);
        if (statusCode != 0) {
            listDestroy(list);
            return statusCode;
        }
    } else {
        statusCode = listConcat(existList, list);
        listDestroy(list);
        if (statusCode != 0) {
            return statusCode;
        }
    }
    return 0;
}

int funcConcat(List **lists, int len, char *string) { // string: "A, B"
    if (strlen(string) != 4)
        return 2;
    List *dest = listsSearch(lists, len, string[0]);
    List *source = listsSearch(lists, len, string[3]);
    if (dest == NULL || source == NULL)
        return 2;
    return listConcat(dest, source);
}

int funcFree(List **lists, int len, char name) {
    int index = listsSearchIndex(lists, len, name);
    if (index < 0)
        return 2;
    List *list;
    int statusCode = listCreate(&list, name);
    if (statusCode != 0)
        return 1;
    listDestroy(lists[index]);
    lists[index] = list;
    return 0;
}

int funcRemove(List **lists, int len, char *string) { // string: "A, 2, 7"
    int index, count;
    int i[2];
    int statusCode = parseInts(i, 2, string + 3);
    if (statusCode != 0) {
        return statusCode;
    }
    index = i[0];
    count = i[1];
    if (count < 1)
        return 2;
    List *list = listsSearch(lists, len, string[0]);
    if (list == NULL)
        return 2;
    if (index < 0 || index + count > list->len)
        return 2;
    if (list->len > index + count) {
        memmove(list->array + index, list->array + index + count, sizeof(int) * (list->len - index - count));
    }
    list->len -= count;
    return 0;
}

int funcCopy(List ***lists, int *len, char *string) { // string: "A, 4, 10, B"
    int a, b;
    int i[2];
    int statusCode = parseInts(i, 2, string + 3);
    if (statusCode != 0) {
        return statusCode;
    }
    a = i[0];
    b = i[1];
    if (a > b || a < 0 || b < 0)
        return 2;
    char destName = string[strlen(string) - 1];
    List *source = listsSearch(*lists, *len, string[0]);
    if (source == NULL || a >= source->len || b >= source->len)
        return 2;
    List *dest = listsSearch(*lists, *len, destName);
    if (dest == NULL) {
        statusCode = listCreate(&dest, destName);
        if (statusCode != 0)
            return statusCode;
        statusCode = listsAdd(lists, len, dest);
        if (statusCode != 0) {
            listDestroy(dest);
            return statusCode;
        }
    }
    for (; a <= b; a++) {
        statusCode = listAdd(dest, source->array[a]);
        if (statusCode != 0) {
            return statusCode;
        }
    }
    return 0;
}

int funcSort(List **lists, int len, char *string) { // string: "A+"
    List *list = listsSearch(lists, len, string[0]);
    if (list == NULL || list->len == 0)
        return 2;
    return listSort(list, string[1] == '+' ? 1 : 0);
}

int funcShuffle(List **lists, int len, char *string) { // string: "A"
    List *list = listsSearch(lists, len, string[0]);
    if (list == NULL)
        return 2;
    for (int x = 0; x < list->len; x++) {
        int randIndex = rand() % list->len;
        int tmp = list->array[x];
        list->array[x] = list->array[randIndex];
        list->array[randIndex] = tmp;
    }
    return 0;
}

int funcStats(List **lists, int len, char *string) { // string: "A"
    List *list = listsSearch(lists, len, string[0]);
    if (list == NULL || list->len == 0)
        return 2;
    List *tmp;
    int statusCode = listCreate(&tmp, '~');
    if (statusCode != 0)
        return statusCode;
    statusCode = listConcat(tmp, list);
    if (statusCode != 0) {
        listDestroy(list);
        return statusCode;
    }
    statusCode = listSort(tmp, 1);
    if (statusCode != 0) {
        listDestroy(list);
        return statusCode;
    }
    int min = tmp->array[0];
    int max = tmp->array[tmp->len - 1];
    int minInd = -1;
    int maxInd = -1;
    for (int x = 0; x < list->len; x++) {
        if (minInd == -1 && list->array[x] == min) {
            minInd = x;
        } else if (maxInd == -1 && list->array[x] == max) {
            maxInd = x;
        }
        if (minInd != -1 && maxInd != -1)
            break;
    }
    int mostOftenInt = 0;
    int mostOftenCount = 0;
    int curInt = tmp->array[0];
    int curCount = 1;
    long double average = (double) min / tmp->len;
    for (int x = 1; x < tmp->len; x++) {
        average += (double) tmp->array[x] / tmp->len;
        if (tmp->array[x] == curInt) {
            curCount++;
        } else {
            if (curCount >= mostOftenCount) {
                if (curCount == mostOftenCount) {
                    if (curInt > mostOftenInt) {
                        mostOftenInt = curInt;
                        mostOftenCount = curCount;
                    }
                } else {
                    mostOftenInt = curInt;
                    mostOftenCount = curCount;
                }
            }
            curInt = tmp->array[x];
            curCount = 1;
        }
    }
    if (tmp->array[tmp->len - 1] == tmp->array[tmp->len - 2]) {
        if (curCount >= mostOftenCount) {
            if (curCount == mostOftenCount) {
                if (curInt > mostOftenInt) {
                    mostOftenInt = curInt;
                }
            } else {
                mostOftenInt = curInt;
            }
        }
    }
    printf("[ %c ]  len: %d  min: %d [%d]  max: %d [%d]\n", list->name, list->len, min, minInd, max, maxInd);
    printf("Most often: %d Average: %ld Max diff: %ld\n",
           mostOftenInt, (long) average,
           average - min > max - average ? (long) (average - min) : (long) (max - average));
    listDestroy(tmp);
    return 0;
}

int funcPrint(List **lists, int len, char *string) { // string: "A, 4, 16" or "A, all"
    List *list = listsSearch(lists, len, string[0]);
    if (list == NULL)
        return 2;
    if (string[3] == 'a') {
        listPrint(list);
    } else {
        int a, b;
        int i[2];
        int statusCode = parseInts(i, 2, string + 3);
        if (statusCode != 0) {
            return statusCode;
        }
        a = i[0];
        b = i[1];
        if (a > b || a < 0 || b < 0 || a >= list->len || b >= list->len)
            return 2;
        printf("[ %c ] ");
        for (; a <= b; a++) {
            printf("%d ", list->array[a]);
        }
        printf("\n");
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        return 69;

    FILE *data;
    List **lists = NULL;
    int len = 0;

    if ((data = fopen(argv[1], "r")) == NULL) {
        return 1;
    }

    int statusCode = 0;
    char *command;
    char *args;
    while (statusCode != -1) {
        char metSep;
        statusCode = readString(data, &command, " ;", &metSep);
        if (statusCode != 0) {
            listsDestroy(lists, len);
            fclose(data);
            return statusCode;
        }
        if (metSep == ';') { // Free(A);
            statusCode = funcFree(lists, len, command[5]);
            free(command);
            if (statusCode != 0) {
                listsDestroy(lists, len);
                fclose(data);
                return statusCode;
            }
            statusCode = nextLine(data);
            continue;
        } else {
            statusCode = readString(data, &args, ";", NULL);
            if (statusCode != 0) {
                free(command);
                listsDestroy(lists, len);
                fclose(data);
                return statusCode;
            }
        }
        if (strcmp(command, "Load") == 0) {
            statusCode = funcLoad(&lists, &len, args);
            free(args);
            free(command);
            if (statusCode != 0) {
                listsDestroy(lists, len);
                fclose(data);
                return statusCode;
            }
        } else if (strcmp(command, "Save") == 0) {
            statusCode = funcSave(lists, len, args);
            free(args);
            free(command);
            if (statusCode != 0) {
                listsDestroy(lists, len);
                fclose(data);
                return statusCode;
            }
        } else if (strcmp(command, "Rand") == 0) {
            statusCode = funcRand(&lists, &len, args);
            free(args);
            free(command);
            if (statusCode != 0) {
                listsDestroy(lists, len);
                fclose(data);
                return statusCode;
            }
        } else if (strcmp(command, "Concat") == 0) {
            statusCode = funcConcat(lists, len, args);
            free(args);
            free(command);
            if (statusCode != 0) {
                listsDestroy(lists, len);
                fclose(data);
                return statusCode;
            }
        } else if (strcmp(command, "Remove") == 0) {
            statusCode = funcRemove(lists, len, args);
            free(args);
            free(command);
            if (statusCode != 0) {
                listsDestroy(lists, len);
                fclose(data);
                return statusCode;
            }
        } else if (strcmp(command, "Copy") == 0) {
            statusCode = funcCopy(&lists, &len, args);
            free(args);
            free(command);
            if (statusCode != 0) {
                listsDestroy(lists, len);
                fclose(data);
                return statusCode;
            }
        } else if (strcmp(command, "Sort") == 0) {
            statusCode = funcSort(lists, len, args);
            free(args);
            free(command);
            if (statusCode != 0) {
                listsDestroy(lists, len);
                fclose(data);
                return statusCode;
            }
        } else if (strcmp(command, "Shuffle") == 0) {
            statusCode = funcShuffle(lists, len, args);
            free(args);
            free(command);
            if (statusCode != 0) {
                listsDestroy(lists, len);
                fclose(data);
                return statusCode;
            }
        } else if (strcmp(command, "Stats") == 0) {
            statusCode = funcStats(lists, len, args);
            free(args);
            free(command);
            if (statusCode != 0) {
                listsDestroy(lists, len);
                fclose(data);
                return statusCode;
            }
        } else if (strcmp(command, "Print") == 0) {
            statusCode = funcPrint(lists, len, args);
            free(args);
            free(command);
            if (statusCode != 0) {
                fclose(data);
                listsDestroy(lists, len);
                return statusCode;
            }
        } else {
            listsDestroy(lists, len);
            fclose(data);
            free(command);
            free(args);
            return 2;
        }
        statusCode = nextLine(data);
    }
    listsDestroy(lists, len);
    fclose(data);
    return 0;
}