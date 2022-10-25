#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {
    unsigned int id;
    unsigned int size;
    char *text;
} message;

typedef message *ListType;

typedef struct {
    ListType *data;
    int size;
    int capacity;
} List;

short addToList(List *list, ListType data) {
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

void destroyList(List *list) {
    for (int x = 0; x < list->size; x++) {
        free(list->data[x]);
    }
    free(list->data);
    free(list);
}

List *createList() {
    List *list = (List *) malloc(sizeof(List));
    ListType *data = (ListType *) malloc(sizeof(ListType) * 5);
    if (list == NULL || data == NULL) {
        return NULL;
    }
    list->size = 0;
    list->data = data;
    list->capacity = 5;
    return list;
}

short genFilename(char **result, char *extension) {
    int len = 11 + strlen(extension);
    char *res = (char *) malloc(sizeof(char) * len);
    if (res == NULL) {
        return 0;
    }
    res[len - 1] = '\0';
    for (int x = 0; x < 10; x++) {
        int ch = rand() % 36;
        if (ch < 10)
            res[x] = ch + '0';
        else
            res[x] = ch - 10 + 'a';
    }
    for (int x = 10; x < len - 1; x++) {
        res[x] = extension[x - 10];
    }
    *result = res;
    return 1;
}

short getMessage(char *res, int resCapacity) {
    char c;
    int count = 0;
    while ((c = getchar()) != '\n') {
        if (count == resCapacity - 1) {
            return 1;
        }
        if (!(
                ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') ||
                ('0' <= c && c <= '9') || (c == ' ') || (c == '.') || (c == ',') ||
                (c == '"') || (c == '?') || (c == ':') || (c == ';') || (c == '-') ||
                (c == '!') || (c == '(') || (c == ')') || (c == '=') || (c == '*')
        )) {
            return 2;
        }
        res[count] = c;
        count++;
    }
    res[count] = '\0';
    return 0;
}

void writeMessage(FILE *file, char *text, int id) {
    char *c;
    c = (char *) &id;
    for (int x = 0; x < sizeof(int); x++) {
        putc(*c++, file);
    }
    int len = strlen(text);
    c = (char *) &len;
    for (int x = 0; x < sizeof(int); x++) {
        putc(*c++, file);
    }
    c = (char *) text;
    for (int x = 0; x < len; x++) {
        putc(*c++, file);
    }
}

short loadMessages(List **result, FILE *in, int count) {
    int id, len;
    List *list = createList();
    if (list == NULL) {
        return 1;
    }
    for (int i = 0; i < count; i++) {
        char *c;
        c = (char *) &id;
        for (int x = 0; x < sizeof(int); x++) {
            *c = getc(in);
            c++;
        }
        len = 0;
        c = (char *) &len;
        for (int x = 0; x < sizeof(int); x++) {
            *c = getc(in);
            c++;
        }
        char *text = (char *) malloc(sizeof(char) * (len + 1));
        if (text == NULL) {
            destroyList(list);
            return 1;
        }
        c = (char *) text;
        for (int x = 0; x < len; x++) {
            *c = getc(in);
            c++;
        }
        *c = '\0';
        message *mes = (message *) malloc(sizeof(message));
        if (mes == NULL) {
            destroyList(list);
            return 2;
        }
        mes->id = id;
        mes->size = len + 1;
        mes->text = text;
        if (!addToList(list, mes)) {
            destroyList(list);
            free(text);
            return 3;
        }
    }
    *result = list;
    return 0;
}

void printList(List *mes) {
    printf("\n\n");
    for (int x = 0; x < mes->size; x++) {
        printf("ID: %d\n", mes->data[x]->id);
        printf("LEN: %d\n", mes->data[x]->size);
        printf("TEXT: %s\n\n", mes->data[x]->text);
    }
}

int main(int argc, char *argv[]) {
    if (argc > 2)
        return 69;

    FILE *data;
    char *filename;
    int id = 0;

    if (!genFilename(&filename, ".csv")) {
        return 1;
    }
    if ((data = fopen(filename, "wb")) == NULL) {
        free(filename);
        return 3;
    }

    char message[100];

    while (1) {
        printf("Enter message %d\n", id + 1);
        switch (getMessage(message, 100)) {
            case 1: free(filename); fclose(data); return 4;
            case 2: free(filename); fclose(data); return 5;
        }
        if (strcmp(message, argv[1]) == 0)
            break;
        writeMessage(data, message, id);
        id++;
    }
    fclose(data);

    if ((data = fopen(filename, "rb")) == NULL) {
        free(filename);
        return 3;
    }
    free(filename);
    List *messages;
    switch (loadMessages(&messages, data, id)) {
        case 1: fclose(data); return 9;
        case 2: fclose(data); return 10;
        case 3: fclose(data); return 11;
    }
    printList(messages);
    destroyList(messages);
    fclose(data);
    return 0;
}