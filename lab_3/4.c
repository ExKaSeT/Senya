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

short parseTmp(FILE *file, char **filename, int *id) {
    int strLen;
    if (fscanf(file, "%d", &strLen) != 1) {
        return 2;
    }
    char *name = (char *) malloc(sizeof(char) * (strLen + 1));
    if (name == NULL) {
        return 1;
    }
    if (fscanf(file, "%d\t%s", id, name) != 2) {
        return 2;
    }
    *filename = name;
    return 0;
}

short isCorrectMessage(const char *message) {
    if (message[0] == '\0')
        return 0;
    short findEnd = 0;
    for (int x = 0; x < 100; x++) {
        if (message[x] == '\0') {
            findEnd = 1;
            break;
        } else if (!(
                ('A' <= message[x] && message[x] <= 'Z') ||
                ('a' <= message[x] && message[x] <= 'z') ||
                ('0' <= message[x] && message[x] <= '9')
        )) {
            return 0;
        }
    }
    if (findEnd)
        return 1;
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
    for (int x = 0; x < mes->size; x++) {
        printf("ID: %d\n", mes->data[x]->id);
        printf("LEN: %d\n", mes->data[x]->size);
        printf("TEXT: %s\n\n", mes->data[x]->text);
    }
}

int main(int argc, char *argv[]) {
    if (argc > 2)
        return 69;
    FILE *tmp, *data;
    int id = 0;
    char *filename;
    if ((tmp = fopen("tmp", "r")) == NULL) {
        if (!genFilename(&filename, ".csv")) {
            fclose(tmp);
            return 1;
        }
    } else {
        switch (parseTmp(tmp, &filename, &id)) {
            case 1:
                return 1;
            case 2:
                return 2;
        }
    }
    fclose(tmp);

    char *message;
    if (argc == 1) {
        char mes[100];
        printf("Enter message:\n");
        scanf("%s", mes);
        message = mes;
    } else {
        message = argv[1];
    }
    if (!isCorrectMessage(message)) {
        free(filename);
        return 5;
    }
    id++;
    if ((tmp = fopen("tmp", "w")) == NULL) {
        free(filename);
        return 6;
    }
    fprintf(tmp, "%d\n%d\t%s", (int) strlen(filename), id, filename);
    fclose(tmp);
    if ((data = fopen(filename, "a+b")) == NULL) {
        free(filename);
        return 3;
    }
    writeMessage(data, message, id);
    fclose(data);
    if (argc == 2) {
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
    } else {
        free(filename);
    }
    return 0;
}