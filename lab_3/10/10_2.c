#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef char ListType;

typedef struct Node Node;
struct Node {
    ListType *data;
    Node *next;
};

typedef struct {
    Node *first;
    Node *last;
    int size;
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

int listInsert(List *list, int index, ListType *data) {
    if (index < -1 || index > list->size)
        return 1;
    Node *newNode = (Node *) malloc(sizeof(Node));
    if (newNode == NULL) {
        return 2;
    }
    newNode->data = data;
    if (list->size == 0) { // 0 элементов
        list->first = newNode;
        list->last = newNode;
        newNode->next = NULL;
    } else if (index == 0) { // вставка в начало
        newNode->next = list->first;
        list->first = newNode;
    } else if (index == -1 || index == list->size) { // вставка в конец
        list->last->next = newNode;
        newNode->next = NULL;
        list->last = newNode;
    } else {
        Node *node = list->first;
        for (int x = 1; x < index; x++) {
            node = node->next;
        }
        newNode->next = node->next;
        node->next = newNode;
    }
    list->size++;
    return 0;
}

void listFreeNode(Node *node) {
    free(node->data);
    free(node);
}

int listRemove(List *list, int index) {
    if (index < -1 || index >= list->size)
        return 1;
    if (list->size < 1)
        return 2;

    if (list->size == 1) { // если 1 элемент
        listFreeNode(list->first);
        list->first = NULL;
    } else if (index == 0) { // удаление из начала
        Node *node = list->first;
        list->first = node->next;
        listFreeNode(node);
    } else if (index == -1 || index == list->size - 1) { // удаление последнего
        Node *prevNode = list->first;
        for (int x = 2; x < list->size; x++) {
            prevNode = prevNode->next;
        }
        listFreeNode(prevNode->next);
        prevNode->next = NULL;
    } else {
        Node *nextNode;
        Node *prevNode = list->first;
        for (int x = 1; x < index; x++) {
            prevNode = prevNode->next;
        }
        nextNode = prevNode->next->next;
        listFreeNode(prevNode->next);
        prevNode->next = nextNode;
    }
    list->size--;
    return 0;
}

void listDestroy(List *list) {
    Node *node = list->first;
    Node *next;
    if (list->size > 0) {
        while (node->next != NULL) {
            next = node->next;
            listFreeNode(node);
            node = next;
        }
        listFreeNode(node);
    }
    free(list);
}

int listExtractFirst(List *list, ListType **get) {
    if (list->size < 1) {
        return 0;
    }
    Node *first = list->first;
    *get = first->data;
    list->first = first->next;
    free(first);
    list->size--;
    return 1;
}


int readString(char **result, char *separators, int (*isCorrectChar)(char)) {
    int resCapacity = 1;
    char *res = (char *) malloc(sizeof(char) * resCapacity);
    if (res == NULL)
        return 1;
    char c;
    int count = 0;
    while (1) {
        c = getchar();
        int x = 0;
        int isBreak = 0;
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
    return 0;
}

int parseInt(const char *string) { // int > 0; 0 - incorrect str
    int x = 0;
    while (string[x] != '\0') {
        if (string[x] < '0' || string[x] > '9')
            return 0;
        x++;
    }
    x = 0;
    while (string[x] == '0')
        x++;
    if (string[x] == '\0')
        return 0;
    int num = string[x] - '0';
    x++;
    while (string[x] != '\0') {
        num = num * 10 + (string[x] - '0');
        x++;
    }
    return num;
}

int main(int argc, char *argv[]) {
    setbuf(stdout, 0);

    if (argc != 2)
        return 69;

    FILE *out;
    int statusCode;
    int lineCount = parseInt(argv[1]);
    if (lineCount == 0)
        return 1;
    int line = 0;
    List *queue;
    List *stack;

    char command;
    char *string, *stringCopy;
    do {
        listCreate(&queue);
        listCreate(&stack);
        printf("Enter strings:\n");

        while (line != lineCount) {
            statusCode = readString(&string, "\n", NULL);
            if (statusCode != 0) {
                listDestroy(queue);
                listDestroy(stack);
                return statusCode;
            }
            statusCode = listInsert(queue, -1, string);
            if (statusCode != 0) {
                free(string);
                listDestroy(queue);
                listDestroy(stack);
                return statusCode;
            }
            stringCopy = (char *) malloc(sizeof(char) * (strlen(string) + 1));
            if (stringCopy == NULL) {
                listDestroy(queue);
                listDestroy(stack);
                return 1;
            }
            strcpy(stringCopy, string);
            statusCode = listInsert(stack, 0, stringCopy);
            if (statusCode != 0) {
                free(stringCopy);
                listDestroy(queue);
                listDestroy(stack);
                return statusCode;
            }
            getchar();
            line++;
        }
        if ((out = fopen("out.txt", "w")) == NULL) {
            listDestroy(queue);
            listDestroy(stack);
            return 777;
        }
        while (listExtractFirst(queue, &string)) {
            fprintf(out, "%s\n", string);
            free(string);
        }
        fclose(out);

        printf("1 - delete last N/2\n0 - exit\n");
        scanf("%c", &command);
        getchar();
        getchar();
        printf("\n");

        if (command == '1') {
            if ((out = fopen("out.txt", "w")) == NULL) {
                listDestroy(queue);
                listDestroy(stack);
                return 777;
            }
            for (int x = 0; x < stack->size / 2; x++) {
                listExtractFirst(stack, &string);
                free(string);
            }
            while (listExtractFirst(stack, &string)) {
                fprintf(out, "%s\n", string);
                free(string);
            }
            fclose(out);
            printf("Done\n0 - exit\n");
            scanf("%c", &command);
            getchar();
            getchar();
            printf("\n");
        }
        listDestroy(queue);
        listDestroy(stack);
        line = 0;
    } while (command != '0');
    return 0;
}