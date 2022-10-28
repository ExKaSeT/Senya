#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>


typedef enum {
    male,
    female
} Sex;

typedef struct {
    char *lastname;
    char *name;
    char *patronymic;
    char *birthDate;
    Sex sex;
    double salary;
} Person;

typedef Person ListType;

typedef struct Node Node;
struct Node {
    ListType *data;
    Node *next;
};

typedef struct {
    Node *first;
    int size;
} List;

short listCreate(List **get) {
    List *list = (List *) malloc(sizeof(List));
    if (list == NULL) {
        return 1;
    }
    list->size = 0;
    list->first = NULL;
    *get = list;
    return 0;
}

short listInsert(List *list, int index, ListType *data) {
    if (index < -1 || index > list->size)
        return 1;
    Node *newNode = (Node *) malloc(sizeof(Node));
    if (newNode == NULL) {
        return 2;
    }
    newNode->data = data;
    if (list->size == 0) { // 0 элементов
        list->first = newNode;
        newNode->next = NULL;
    } else if (index == 0) { // вставка в начало
        newNode->next = list->first;
        list->first = newNode;
    } else if (index == -1 || index == list->size) { // вставка в конец
        Node *node = list->first;
        while (node->next != NULL) {
            node = node->next;
        }
        node->next = newNode;
        newNode->next = NULL;
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

void listRemoveNode(Node *node) {
    free(node->data);
    free(node);
}

short listRemove(List *list, int index) {
    if (index < -1 || index >= list->size)
        return 1;
    if (list->size < 1)
        return 2;

    if (list->size == 1) { // если 1 элемент
        listRemoveNode(list->first);
        list->first = NULL;
    } else if (index == 0) { // удаление из начала
        Node *node = list->first;
        list->first = node->next;
        listRemoveNode(node);
    } else if (index == -1 || index == list->size - 1) { // удаление последнего
        Node *prevNode = list->first;
        for (int x = 2; x < list->size; x++) {
            prevNode = prevNode->next;
        }
        listRemoveNode(prevNode->next);
        prevNode->next = NULL;
    } else {
        Node *nextNode;
        Node *prevNode = list->first;
        for (int x = 1; x < index; x++) {
            prevNode = prevNode->next;
        }
        nextNode = prevNode->next->next;
        listRemoveNode(prevNode->next);
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
            listRemoveNode(node);
            node = next;
        }
        listRemoveNode(node);
    }
    free(list);
}

short listHasNextNode(List *list, Node *node) {
    if (node->next != NULL)
        return 1;
    return 0;
}

void listNextNode(List *list, Node **node) {
    if (*node == NULL) {
        *node = list->first;
    } else {
        *node = (*node)->next;
    }
}

void printPersons(List *list) {
    if (list->size == 0) {
        printf(":/");
        return;
    }
    Node *node = NULL;
    do {
        listNextNode(list, &node);
        Person *person = node->data;
        printf("\n------------------------------\n%s\n%s\n%s\n%s\n%d\n%f\n------------------------------\n",
               person->lastname, person->name, person->patronymic,
               person->birthDate, person->sex, person->salary);
    } while (listHasNextNode(list, node));
}

void freeAll(int count, ...) {
    va_list args;
    void *ptr;
    va_start(args, count);
    for (int x = 0; x < count; x++) {
        ptr = va_arg(args, void *);
        free(ptr);
    }

//    void *ptr = (&count + 1);
//    int *ptr = (&count + 1);
//    for (int x = 0; x < count; x++) {
//        free(ptr);
//        ptr++;
//    }
}

short isLetter(char c) {
    return (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'));
}

short isNumOrDot(char c) {
    return (('0' <= c && c <= '9') || c == '.');
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
        if (!isCorrectChar(c)) {
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

short isCorrectSex(char *string) {
    if (strcmp(string, "male") == 0)
        return 1;
    if (strcmp(string, "female") == 0)
        return 2;
    return 0;
}

short isCorrectDouble(char *string, double *result) {
    char *endPtr;
    errno = 0;
    *result = strtod(string, &endPtr);
    if (errno != 0 || *endPtr != '\0')
        return 0;
    return 1;
}

typedef struct {
    int day;
    int month;
    int year;
} Date;

Date parseDate(const char *date) { // correct date
    int day;
    int x = 0;
    day = (date[x] == '0') ? (date[x + 1] - '0') : (date[x] - '0') * 10 + (date[x + 1] - '0');

    int month;
    x = 3;
    month = (date[x] == '0') ? (date[x + 1] - '0') : (date[x] - '0') * 10 + (date[x + 1] - '0');

    int year;
    x = 6;
    year = (date[x] - '0');
    x++;
    while (date[x] != '\0') {
        year = year * 10 + (date[x] - '0');
        x++;
    }
    Date d = {day, month, year};
    return d;
}

short isCorrectDate(const char *string) { // string contains nums or dots
    int day, month, year;
    int dotCount = 0;
    int x = 0;
    while (string[x] != '\0') {
        if (string[x] == '.') {
            if (x != 2 && x != 5)
                return 0;
            dotCount++;
        }
        x++;
    }
    if (x != 10)
        return 0;
    if (dotCount != 2)
        return 0;
    if (string[6] == '0') // первая цифра года
        return 0;
    Date date = parseDate(string);
    day = date.day;
    month = date.month;
    year = date.year;
    if (day < 1)
        return 0;
    if (month < 1 || month > 12)
        return 0;
    if (year < 2000)
        return 0;
    if (month == 2) {
        int leap = 0;
        if (((year % 4 == 0) && (year % 100 != 0)) || year % 400 == 0) {
            leap = 1;
        }
        if (!leap && day <= 28) {
            return 1;
        } else if (leap && day <= 29) {
            return 1;
        }
        return 0;
    }
    if (month < 8) {
        if (day <= month % 2 + 30)
            return 1;
        return 0;
    } else {
        if (day <= 31 - month % 2)
            return 1;
        return 0;
    }
}

short readPerson(FILE *file, Person **result) {
    short isLastPerson;
    char *lastname, *name, *patronymic, *date, *sexString, *salaryString;
    double salary;
    Sex sex;
    short statusCode;
    statusCode = readString(file, &lastname, "\t \n", isLetter);
    if (statusCode != 0) {
        return statusCode;
    }
    statusCode = readString(file, &name, "\t \n", isLetter);
    if (statusCode != 0) {
        free(lastname);
        return statusCode;
    }
    statusCode = readString(file, &patronymic, "\t \n", isLetter);
    if (statusCode != 0) {
        freeAll(2, lastname, name);
        return statusCode;
    }
    statusCode = readString(file, &date, "\t \n", isNumOrDot);
    if (statusCode != 0) {
        freeAll(3, lastname, name, patronymic);
        return statusCode;
    }
    if (!isCorrectDate(date)) {
        freeAll(4, lastname, name, patronymic, date);
        return 4;
    }
    statusCode = readString(file, &sexString, "\t \n", isLetter);
    if (statusCode != 0) {
        freeAll(4, lastname, name, patronymic, date);
        return statusCode;
    }
    switch (isCorrectSex(sexString)) {
        case 0:
            freeAll(5, lastname, name, patronymic, date, sexString);
            return 5;
        case 1:
            sex = male;
            break;
        case 2:
            sex = female;
            break;
    }
    statusCode = readString(file, &salaryString, "\t \n", isNumOrDot);
    if (statusCode != 0) {
        if (statusCode == -1) { // EOF
            isLastPerson = 1;
        } else {
            freeAll(5, lastname, name, patronymic, date, sexString);
            return statusCode;
        }
    }
    if (!isCorrectDouble(salaryString, &salary)) {
        freeAll(6, lastname, name, patronymic, date, sexString, salaryString);
        return 6;
    }
    Person *person = (Person *) malloc(sizeof(Person));
    if (person == NULL) {
        freeAll(6, lastname, name, patronymic, date, sexString, salaryString);
        return 7;
    }
    person->sex = sex;
    person->lastname = lastname;
    person->name = name;
    person->patronymic = patronymic;
    person->birthDate = date;
    person->salary = salary;
    *result = person;
    freeAll(2, salaryString, sexString);
    if (isLastPerson)
        return -1; // EOF
    return 0;
}

short compareByBirthDate(Person *person1, Person *person2) {
    Date date1 = parseDate(person1->birthDate);
    Date date2 = parseDate(person2->birthDate);
    if (date1.year < date2.year) {
        return 1;
    } else if (date1.year > date2.year) {
        return -1;
    } else {
        if (date1.month < date2.month) {
            return 1;
        } else if (date1.month > date2.month) {
            return -1;
        } else {
            if (date1.day < date2.day) {
                return 1;
            } else if (date1.day > date2.day) {
                return -1;
            }
        }
    }
    return 0;
}

short readPersonsSorted(FILE *file, List **result, short (*compareTo)(Person *, Person *)) {
    Person *person;
    short statusCode = 0;
    List *list;
    if (listCreate(&list))
        return 10;
    while (statusCode != -1) { // -1 = EOF
        statusCode = readPerson(file, &person);
        if (statusCode != 0 && statusCode != -1) {
            listDestroy(list);
            return statusCode;
        }
        if (list->size == 0) {
            switch (listInsert(list, 0, person)) {
                case 1:
                    free(person);
                    listDestroy(list);
                    return 8;
                case 2:
                    free(person);
                    listDestroy(list);
                    return 9;
            }
            continue;
        }
        Node *node = NULL;
        int x = 0;
        do {
            listNextNode(list, &node);
            if (compareTo(person, node->data) == 1) {
                switch (listInsert(list, x, person)) {
                    case 1:
                        free(person);
                        listDestroy(list);
                        return 8;
                    case 2:
                        free(person);
                        listDestroy(list);
                        return 9;
                }
                break;
            }
            x++;
        } while (listHasNextNode(list, node));
        if (list->size == x) { // <= предыдущих
            switch (listInsert(list, x, person)) {
                case 1:
                    free(person);
                    listDestroy(list);
                    return 8;
                case 2:
                    free(person);
                    listDestroy(list);
                    return 9;
            }
        }
    }
    *result = list;
    return 0;
}

short saveToFile(char *filename, List *list) {
    if (list->size == 0)
        return 1;
    FILE *file;
    if ((file = fopen(filename, "w")) == NULL) {
        return 2;
    }
    Node *node = NULL;
    do {
        listNextNode(list, &node);
        Person *person = node->data;
        if (fprintf(file, "%s\t%s\t%s\t%s\t%s\t%f\n", person->lastname,
                    person->name, person->patronymic, person->birthDate,
                    person->sex == male ? "male" : "female", person->salary) < 0) {
            fclose(file);
            return 3;
        }
    } while (listHasNextNode(list, node));
    fclose(file);
    return 0;
}

int searchPerson(List *list, char *lastname, char *name, char *patronymic,
                 char *birthDate, Sex *sex, double *salary) {
    Node *node = NULL;
    int x = -1;
    do {
        x++;
        listNextNode(list, &node);
        Person *person = node->data;
        if (lastname != NULL)
            if (strcmp(lastname, person->lastname) != 0)
                continue;
        if (name != NULL)
            if (strcmp(name, person->name) != 0)
                continue;
        if (patronymic != NULL)
            if (strcmp(patronymic, person->patronymic) != 0)
                continue;
        if (birthDate != NULL)
            if (strcmp(birthDate, person->birthDate) != 0)
                continue;
        if (sex != NULL)
            if (*sex != person->sex)
                continue;
        if (salary != NULL)
            if (*salary != person->salary)
                continue;
        return x;
    } while (listHasNextNode(list, node));
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        return 69;

    FILE *data;
    List *list;
    char *filename = argv[1];

    if ((data = fopen(filename, "r")) == NULL) {
        return 1;
    }

    switch (readPersonsSorted(data, &list, compareByBirthDate)) {
        case 0:
            break;
        default:
            return 69;
    }
    printPersons(list);
    printf("\nINDEX: %d\n", searchPerson(list, "Irbitsky", NULL, NULL, NULL, NULL, NULL));
    switch (listRemove(list, 0)) {
        case 0:
            break;
        default:
            return 69;
    }
    printPersons(list);
    switch (saveToFile("out.txt", list)) {
        case 0:
            break;
        default:
            return 69;
    }

    listDestroy(list);
    fclose(data);
    return 0;
}