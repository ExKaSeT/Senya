#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>


typedef struct Employee {
    unsigned int id;
    char name[80];
    char surname[80];
    double salary;
} Employee;

typedef struct Node Node;
struct Node {
    Employee *key;
    Node *parent;
    Node *child;
    Node *left;
    Node *right;
    int degree;
};

typedef struct Heap {
    int size;
    Node *root;
} Heap;

void swap(Node **a, Node **b) {
    Node *tmp = *a;
    *a = *b;
    *b = tmp;
}

void printEmployees(Employee *list, int size) {
    for (int x = 0; x < size; x++) {
        printf("%u %s %s %lf\n", list[x].id, list[x].name, list[x].surname, list[x].salary);
    }
}

void printEmployee(FILE *stream, Employee toPrint) {
    fprintf(stream, "%u %s %s %lf\n", toPrint.id, toPrint.name, toPrint.surname, toPrint.salary);
}

int isValidStr(char *str) {
    int x = 0;
    while (str[x] != '\0') {
        if (!isalpha(str[x])) {
            return 0;
        }
        x++;
    }
    return 1;
}

int isValid(unsigned int *ids, int idsSize, char *name, char *surname, double salary) {
    unsigned int toCheck = ids[idsSize];
    for (int x = 0; x < idsSize; x++) {
        if (ids[x] == toCheck) {
            return 0;
        }
    }
    if (!isValidStr(name) || !isValidStr(surname)) {
        return 0;
    }
    if (salary < 0) {
        return 0;
    }
    return 1;
}

Employee *readEmployees(FILE *fi, int *currentSize, int *statusCode) {
    if (!fi) {
        (*statusCode) = 2;
        return NULL;
    }
    int size = 5;
    (*currentSize) = 0;
    Employee *employeeList = (Employee *) malloc(sizeof(Employee) * size);
    Employee *tmpEmployee = NULL;
    if (!employeeList) {
        (*statusCode) = 1;
        return NULL;
    }

    unsigned int id = 0, *tmpIds;
    unsigned int *ids = (unsigned int *) malloc(sizeof(unsigned int) * size);
    if (!ids) {
        free(employeeList);
        (*statusCode) = 1;
        return NULL;
    }

    double salary = 0;
    char ch = 0;
    while (!feof(fi)) {
        if ((*currentSize) >= size - 1) {
            size *= 2;
            if (!(tmpEmployee = (Employee *) realloc(employeeList, sizeof(Employee) * size)) ||
                !(tmpIds = (unsigned int *) realloc(ids, sizeof(unsigned int) * size))) {
                free(employeeList);
                free(ids);
                (*statusCode) = 1;
                return NULL;
            }
            employeeList = tmpEmployee;
            ids = tmpIds;
        }

        if (fscanf(fi, "%u %s %s %lf\n", &id, employeeList[(*currentSize)].name,
                   employeeList[(*currentSize)].surname, &salary) != 4) {
            while (ch != '\n')
                ch = fgetc(fi);
            continue;
        }

        ids[(*currentSize)] = id;
        if (!isValid(ids, (*currentSize), employeeList[(*currentSize)].name,
                     employeeList[(*currentSize)].surname, salary)) {
            continue;
        }
        employeeList[(*currentSize)].id = id;
        employeeList[(*currentSize)++].salary = salary;
    }

    if (!(tmpEmployee = (Employee *) realloc(employeeList, sizeof(Employee) * (*currentSize)))) {
        free(employeeList);
        free(ids);
        (*statusCode) = 1;
        return NULL;
    }
    employeeList = tmpEmployee;
    free(ids);
    (*statusCode) = 0;
    return employeeList;
}

void initHeap(Heap *heap) {
    heap->root = NULL;
    heap->size = 0;
}

int insertNode(int compare, Heap *heap, Employee *toInsert) {
    if (compare != 1 && compare != -1) {
        return 5;
    }
    if (!heap) {
        return 2;
    }
    Node *newNode = (Node *) malloc(sizeof(Node));
    if (!newNode) {
        return 1;
    }
    newNode->key = toInsert;
    newNode->child = NULL;
    newNode->degree = 0;
    newNode->parent = NULL;

    if (heap->size == 0) {
        heap->root = newNode;
        heap->root->right = newNode;
        heap->root->left = newNode;
    } else {
        Node *prevRight = heap->root->right;
        heap->root->right = newNode;
        newNode->left = heap->root;
        newNode->right = prevRight;
        prevRight->left = newNode;
    }
    if (compare == -1 && (newNode->key->salary < heap->root->key->salary) ||
        compare == 1 && (newNode->key->salary > heap->root->key->salary)) {
        heap->root = newNode;
    }
    heap->size++;
    return 0;
}

void heapLink(Heap *heap, Node *parentNode, Node *childNode) {
    childNode->left->right = childNode->right;
    childNode->right->left = childNode->left;
    childNode->left = NULL;
    childNode->right = NULL;
    parentNode->degree++;
    if (parentNode->child != NULL) {
        childNode->parent = parentNode;
        childNode->right = parentNode->child->right;
        childNode->left = parentNode->child;
        parentNode->child->right->left = childNode;
        parentNode->child->right = childNode;
    } else {
        parentNode->child = childNode;
        childNode->parent = parentNode;
        childNode->left = childNode;
        childNode->right = childNode;
    }
}

int heapConsolidate(int compare, Heap *heap) {
    if (compare != 1 && compare != -1) {
        return 5;
    }
    int listSize = log2(heap->size) + 1;
    Node **listOfTreesDegrees = (Node **) calloc(listSize, sizeof(Node *));
    if (!listOfTreesDegrees)
        return 1;
    for (int x = 0; x < listSize; x++) {
        listOfTreesDegrees[x] = NULL;
    }

    Node *currNode = heap->root, *anotherNode = NULL;
    int currDegree = currNode->degree;
    do {
        while (listOfTreesDegrees[currDegree] != NULL) {
            anotherNode = listOfTreesDegrees[currDegree];
            if (anotherNode == currNode) {
                break;
            }
            if ((compare == -1 && (currNode->key->salary > anotherNode->key->salary)) ||
                compare == 1 && (currNode->key->salary < anotherNode->key->salary)) {
                swap(&currNode, &anotherNode);
            }
            heapLink(heap, currNode, anotherNode);
            listOfTreesDegrees[currDegree] = NULL;
            currDegree++;
        }
        if (((compare == -1 && (currNode->key->salary < heap->root->key->salary)) ||
             compare == 1 && (currNode->key->salary > heap->root->key->salary)) && currNode != heap->root) {
            heap->root = currNode;
        }
        if (anotherNode != currNode)
            listOfTreesDegrees[currDegree] = currNode;

        currNode = currNode->right;
        currDegree = currNode->degree;
    } while (currNode != heap->root);

    heap->root = NULL;
    for (int x = 0; x < listSize; x++) {
        currNode = listOfTreesDegrees[x];
        if (currNode != NULL) {
            if (heap->root == NULL) {
                heap->root = currNode;
                heap->root->left = currNode;
                heap->root->right = currNode;
            } else {
                currNode->left = heap->root;
                currNode->right = heap->root->right;
                heap->root->right->left = currNode;
                heap->root->right = currNode;

                if ((compare == -1 && (currNode->key->salary < heap->root->key->salary)) ||
                    compare == 1 && (currNode->key->salary > heap->root->key->salary)) {
                    heap->root = currNode;
                }
            }
        }
    }
    free(listOfTreesDegrees);
    return 0;
}

int extractRootNode(int compare, Heap *heap, Node **extracted) {
    if (compare != 1 && compare != -1) {
        return 5;
    }
    Node *root = heap->root;
    if (root != NULL) {
        if (root->child != NULL) {
            Node *currNode = root->child;
            Node *nextNode = NULL;
            do {
                nextNode = currNode->right;
                currNode->parent = NULL;
                currNode->left = root;
                currNode->right = root->right;
                root->right->left = currNode;
                root->right = currNode;
                currNode = nextNode;
            } while (currNode != root->child);
        }
        root->left->right = root->right;
        root->right->left = root->left;

        if (root->right == root) {
            heap->root = NULL;
        } else {
            int statusCode;
            heap->root = root->right;
            statusCode = heapConsolidate(compare, heap);
            if (statusCode != 0) {
                return statusCode;
            }
        }
        heap->size--;
    }
    (*extracted) = root;
    return 0;
}

int heapSort(int compare, Employee *list, int arrSize, FILE *fi) {
    if (compare != 1 && compare != -1) {
        return 5;
    }
    Heap heap;
    initHeap(&heap);
    int x;
    int statusCode;
    for (x = 0; x < arrSize; x++) {
        statusCode = insertNode(compare, &heap, &(list[x]));
        if (statusCode != 0) {
            return statusCode;
        }
    }
    Node *extracted = NULL;
    for (x = 0; x < arrSize; x++) {
        statusCode = extractRootNode(compare, &heap, &extracted);
        if (statusCode != 0) {
            return statusCode;
        }
        printEmployee(fi, *(extracted->key));
        free(extracted);
        extracted = NULL;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        return 777;
    }

    FILE *in, *out;
    if (!(in = fopen(argv[1], "r")) || !(out = fopen(argv[3], "w"))) {
        return 888;
    }

    int statusCode;
    int count = 0;
    Employee *employeeList = readEmployees(in, &count, &statusCode);
    if (statusCode != 0) {
        fclose(in);
        fclose(out);
        return statusCode;
    }
    printEmployees(employeeList, count);

    if (!strcmp(argv[2], "-a") || !strcmp(argv[2], "/a")) {
        statusCode = heapSort(1, employeeList, count, out);
        if (statusCode != 0) {
            free(employeeList);
            fclose(in);
            fclose(out);
            return statusCode;
        }
    } else if (!strcmp(argv[2], "-d") || !strcmp(argv[2], "/d")) {
        statusCode = heapSort(-1, employeeList, count, out);
        if (statusCode != 0) {
            free(employeeList);
            fclose(in);
            fclose(out);
            return statusCode;
        }
    } else {
        free(employeeList);
        fclose(in);
        fclose(out);
        return -777;
    }

    fclose(in);
    fclose(out);
    free(employeeList);
    return 0;
}