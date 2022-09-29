#include <stdlib.h>
#include <stdio.h>


void error(char* message) {
    printf("%s\n", message);
    exit(1);
}

typedef struct List List;
struct List {
    int* data;
    int size;
    int capacity;
    int resize;
    void (*add) (List*, int);
    void (*destroy) (List*);
};

void addNumToList(List* list, int num) {
    if (list->size == list->capacity) {
        int* newData = (int*) realloc(list->data, sizeof(int)*(list->capacity + list->resize));
        if (newData == NULL) {
            error("Error: Insufficient memory");
        }
        list->data = newData;
        list->capacity += list->resize;
    }
    list->data[list->size] = num;
    list->size++;
}

void destroyList(List* list) {
    free(list->data);
    free(list);
}

List* createList() {
    List* list = (List*) malloc(sizeof(List));
    int* data = (int*) malloc(sizeof(int) * 5);
    if (list == NULL && data == NULL) {
        error("Error: Insufficient memory");
    }
    list->size = 0;
    list->data = data;
    list->capacity = 5;
    list->resize = 5;
    list->add = addNumToList;
    list->destroy = destroyList;
    return list;
}

int length(int number) {
    if (number < 0) {
        number *= -1;
    }
    int count = 0;
    while (number) {
        number /= 10;
        count++;
    }
    return count;
}

int charToInt(char number) {
    if ('0' <= number && number <= '9') {
        return number - '0';
    }
    error("Error: cant cast char to int");
    return -1;
}

// строка это полностью число до '\0' (если он есть)
int stringToInt(char* string, int stringLen) {
    int isFindEnd = 0;
    int oldStringLen = stringLen;
    for (int x = 0; x < oldStringLen; x++) {
        if (isFindEnd) {
            stringLen -= (oldStringLen - x);
            break;
        } else if (string[x] == '\0') {
            isFindEnd = 1;
            stringLen--;
        }
    }
    stringLen;
    int isPositive;
    int index = 0;
    if (string[0] == '-') {
        isPositive = 0;
        index = 1;
    } else {
        isPositive = 1;
    }
    int maxIntLen = length(INT_MAX);
    if (stringLen - index > maxIntLen) {
        error("Error: cant cast string to int");
    }
    int number = charToInt(string[index]);
    for (index++; index < stringLen; index++) {
        int num = charToInt(string[index]);
        if ((double) number <= (INT_MAX - num) / 10.0) {
            number = number * 10 + num;
        } else {
            error("Error: cant cast string to int");
        }
    }
    if (!isPositive) {
        number *= -1;
    }
    return number;
}

void printList(List* list) {
    printf("[ ");
    for (int i = 0; i < list->size; i++) {
        printf("%d, ", list->data[i]);
    }
    printf("\b\b ]\n");
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("Enter path to file and command like arguments");
        return 0;
    } else if (argc != 3) {
        error("Error: incorrect number of arguments");
    }
    char command = argv[2][0];
    if (!('a' <= command && command <= 'e')) {
        error("Error: incorrect command");
    }

    List* list = createList();
    FILE* file = fopen("8.txt", "r");
    if (!file) {
        error("Error: can't open file");
    }

    int stringLen = length(INT_MAX) + 2;
    char string[stringLen];
    for (int x = 0; x < stringLen; x++) {
        string[x] = '\0';
    }
    while (fscanf(file, "%s\n", string) != EOF) {
        if (
                (string[0] == '-' && string[stringLen - 1] == '\0') ||
                (string[0] != '-' && string[stringLen - 2] == '\0')
                ) {
            list->add(list, stringToInt(string, stringLen));
            string[stringLen - 1] = '\0';
            string[stringLen - 2] = '\0';
        } else {
            error("Error: incorrect integer in file");
        }
    }

    printList(list);
    int currentIndex;
    if ('c' <= command && command <= 'e') {
        printf("Enter index of element: ");
        scanf("%d", &currentIndex);
        if (currentIndex < 0 && currentIndex >= list->size) {
            error("Error: incorrect index");
        }
    }

    List* result = createList();
    switch (command) {
        case 'a': {
            for (int x = 0; x < list->size; x += 2) {
                result->add(result, list->data[x]);
            }
            break;
        }
        case 'b': {
            for (int x = 0; x < list->size; x++) {
                if (list->data[x] % 2 == 0) {
                    result->add(result, list->data[x]);
                }
            }
            break;
        }
        case 'c': {
            int maxDifference = 0;
            int intMaxDif;
            for (int x = 0; x < list->size; x++) {
                if (abs(list->data[currentIndex] - list->data[x]) > maxDifference) {
                    maxDifference = abs(list->data[currentIndex] - list->data[x]);
                    intMaxDif = list->data[x];
                }
            }
            result->add(result, intMaxDif);
            break;
        }
        case 'd': {
            int sum = 0;
            for (int x = 0; x < currentIndex; x++) {
                sum += list->data[x];
            }
            result->add(result, sum);
            break;
        }
        case 'e': {
            int sum = 0;
            for (int x = 0; x < list->size; x++) {
                if (list->data[x] < list->data[currentIndex]) {
                    sum += list->data[x];
                }
            }
            result->add(result, sum);
            break;
        }
        default: {
            error("?????");
        }
    }

    printList(result);

    list->destroy(list);
    result->destroy(result);
    return 0;
}

