#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


typedef struct BstNode BstNode;
struct BstNode {
    int key;
    BstNode *left;  // < key
    BstNode *right; // >= key
    BstNode *parent;
    char *string;
};

BstNode *bstMin(BstNode *node) {
    if (node->left == NULL)
        return node;
    return bstMin(node->left);
}

BstNode *bstMax(BstNode *node) {
    if (node->right == NULL)
        return node;
    return bstMax(node->right);
}

BstNode *bstNext(BstNode *node) {
    if (node->right != NULL)
        return bstMin(node->right);
    BstNode *tmp = node->parent;
    while (tmp != NULL && node == tmp->right) {
        node = tmp;
        tmp = tmp->parent;
    }
    return tmp;
}

BstNode *bstPrev(BstNode *node) {
    if (node->left != NULL)
        return bstMax(node->left);
    BstNode *tmp = node->parent;
    while (tmp != NULL && node == tmp->left) {
        node = tmp;
        tmp = tmp->parent;
    }
    return tmp;
}

void bstInsertNode(BstNode *root, BstNode *node) {
    while (1) {
        if (node->key >= root->key) {
            if (root->right != NULL) {
                root = root->right;
            } else {
                node->left = NULL;
                node->right = NULL;
                node->parent = root;
                root->right = node;
                break;
            }
        } else {
            if (root->left != NULL) {
                root = root->left;
            } else {
                node->left = NULL;
                node->right = NULL;
                node->parent = root;
                root->left = node;
                break;
            }
        }
    }
}

int bstInsert(BstNode *root, int key, char *string) {
//    char *str = (char *) malloc(sizeof(char) * 20);
//    strcpy(str, string);
//    string = str;

    BstNode *node = (BstNode *) malloc(sizeof(BstNode));
    if (node == NULL)
        return 1;
    node->key = key;
    node->string = string;
    bstInsertNode(root, node);
    return 0;
}

int bstCreate(BstNode **result, int key, char *string) {
    BstNode *node = (BstNode *) malloc(sizeof(BstNode));
    if (node == NULL)
        return 1;
    node->key = key;
    node->string = string;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    *result = node;
    return 0;
}

void bstDeleteNode(BstNode *node) {
    BstNode *p = node->parent;
    if (node->left == NULL && node->right == NULL) {
        if (p->left == node)
            p->left = NULL;
        if (p->right == node)
            p->right = NULL;
    } else if (node->left == NULL || node->right == NULL) {
        if (node->left == NULL) {
            if (p->left == node)
                p->left = node->right;
            else
                p->right = node->right;
            node->right->parent = p;
        } else {
            if (p->left == node)
                p->left = node->left;
            else
                p->right = node->left;
            node->left->parent = p;
        }
    } else {
        BstNode *successor = bstNext(node);
        node->key = successor->key;
        if (successor->parent->left == successor) {
            successor->parent->left = successor->right;
            if (successor->right != NULL)
                successor->right->parent = successor->parent;
        } else {
            successor->parent->right = successor->right;
            if (successor->right != NULL)
                successor->right->parent = successor->parent;
        }
    }
}

void findWordCountRec(BstNode *root, char *string, int *count) {
    if (*count != 0)
        return;
    if (root != NULL) {
        findWordCountRec(root->left, string, count);
        if (strcmp(string, root->string) == 0) {
            *count = root->key;
            return;
        }
        findWordCountRec(root->right, string, count);
    }
}

int findWordCount(BstNode *root, char *string) {
    int count = 0;
    findWordCountRec(root, string, &count);
    return count;
}

int stringsCompare(char *str1, char *str2) {
    int len = strlen(str1) - strlen(str2);
    if (len > 0) {
        return 1;
    } else if (len < 0) {
        return -1;
    }
    return strcmp(str1, str2);
}

void findMinMaxWordsRec(BstNode *root, char **strings) {
    if (root != NULL) {
        findMinMaxWordsRec(root->left, strings);
        if (stringsCompare(root->string, strings[0]) < 0)
            strings[0] = root->string;
        if (stringsCompare(root->string, strings[1]) > 0)
            strings[1] = root->string;
        findMinMaxWordsRec(root->right, strings);
    }
}

int findMinMaxWords(BstNode *root, char ***result) {
    char **res = (char **) malloc(sizeof(char *) * 2);
    if (res == NULL)
        return 1;
    findMinMaxWordsRec(root, res);
    *result = res;
    return 0;
}

int findCommonestWords(char ***result, BstNode *root, int count) {
    if (root == NULL)
        return 2;
    char **res = (char **) malloc(sizeof(char *) * count);
    if (res == NULL)
        return 1;
    root = bstMax(root);
    res[0] = root->string;
    for (int x = 1; x <= count; x++) {
        root = bstPrev(root);
        if (root == NULL) {
            free(res);
            return 2;
        }
        res[x] = root->string;
    }
    *result = res;
    return 0;
}

void bstPrintRec(BstNode *root, int space) {
    if (root == NULL)
        return;
    space += 1;
    bstPrintRec(root->right, space);
    printf("\n");
    for (int i = 1; i < space; i++)
        printf("\t");
    printf("%d\n", root->key);
    bstPrintRec(root->left, space);
}

void bstPrint(BstNode *root) {
    bstPrintRec(root, 0);
}

void freeAll(int count, ...) {
    va_list args;
    void *ptr;
    va_start(args, count);
    for (int x = 0; x < count; x++) {
        ptr = va_arg(args, void *);
        free(ptr);
    }
}

typedef struct TrieNode TrieNode;
struct TrieNode {
    char key;
    TrieNode **kids;
    int len;
    int capacity;
    BstNode *link;
};

int trieCreate(TrieNode **result) {
    TrieNode *res = (TrieNode *) malloc(sizeof(TrieNode));
    if (res == NULL)
        return 1;
    res->capacity = 5;
    TrieNode **kids = (TrieNode **) malloc(sizeof(TrieNode *) * res->capacity);
    if (kids == NULL) {
        free(res);
        return 1;
    }
    res->kids = kids;
    res->len = 0;
    *result = res;
    return 0;
}

void trieDestroy(TrieNode *root) {
    for (int x = 0; x < root->len; x++) {
        trieDestroy(root->kids[x]);
    }
    freeAll(2, root->kids, root);
}

int trieInsert(TrieNode *root, char *string, BstNode *link) {
    if (string[0] == '\0')
        return 2;
    TrieNode *node = root;
    for (int x = 0; x < strlen(string); x++) {
        char key = string[x];
        int low = 0;
        int high = node->len - 1;
        int isFound = 0;
        int mid;

        while (low <= high) {
            mid = (low + high) >> 1;
            char midVal = node->kids[mid]->key;

            if (midVal < key) {
                low = mid + 1;
            } else if (midVal > key) {
                high = mid - 1;
            } else {
                isFound = 1;
                break;
            }
        }
        if (isFound) {
            node = node->kids[mid];
        } else {
            if (node->len == node->capacity) {
                node->capacity *= 2;
                TrieNode **kids = (TrieNode **) realloc(node->kids, sizeof(TrieNode *) * node->capacity);
                if (kids == NULL) {
                    node->capacity /= 2;
                    return 1;
                }
                node->kids = kids;
            }
            TrieNode *newNode = (TrieNode *) malloc(sizeof(TrieNode));
            if (newNode == NULL) {
                return 1;
            }
            newNode->capacity = 5;
            newNode->len = 0;
            newNode->key = key;
            newNode->link = NULL;
            TrieNode **newKids = (TrieNode **) malloc(sizeof(TrieNode *) * newNode->capacity);
            if (newKids == NULL) {
                free(newNode);
                return 1;
            }
            newNode->kids = newKids;
            // low - index where insert newNode
            if (low != node->len) { // check sizeof(node->kids) ?? TrieNode * or array
                memmove(node->kids + low, node->kids + low + 1, sizeof(node->kids) * (node->len - low));
            }
            node->kids[low] = newNode;
            node->len++;
            node = newNode;

        }
    }
    node->link = link;
    return 0;
}

TrieNode *trieSearch(TrieNode *root, const char *string) {
    if (string[0] == '\0')
        return NULL;
    TrieNode *node = root;
    for (int x = 0; x < strlen(string); x++) {
        char key = string[x];
        int low = 0;
        int high = node->len - 1;
        int isFound = 0;
        int mid;

        while (low <= high) {
            mid = (low + high) >> 1;
            char midVal = node->kids[mid]->key;

            if (midVal < key) {
                low = mid + 1;
            } else if (midVal > key) {
                high = mid - 1;
            } else {
                isFound = 1;
                break;
            }
        }
        if (!isFound)
            return NULL;
        node = node->kids[mid];
    }
    return node;
}

void triePrintRec(TrieNode *root, char *str, int level) {
    if (root->len == 0) {
        str[level] = root->key;
        str[level + 1] = '\0';
        printf("%s\n", str);
    }
    for (int x = 0; x < root->len; x++) {
        str[level] = root->key;
        triePrintRec(root->kids[x], str, level + 1);
    }
}

void triePrint(TrieNode *root) {
    char word[100];
    for (int x = 0; x < root->len; x++)
        triePrintRec(root->kids[x], word, 0);
}



int main(int argc, char *argv[]) {
    if (argc != 2)
        return 69;

    FILE *data;
    char *filename = argv[1];

    TrieNode *t;
    trieCreate(&t);
    char string[40];
    char string2[40];
    strcpy(string, "qwerty");
    strcpy(string2, "qwop");
    trieInsert(t, string, NULL);
    trieInsert(t, string2, NULL);
    triePrint(t);

    printf("%c\n", trieSearch(t, "qwerty")->key);


//    BstNode *bst;
//    char *str = (char *) malloc(sizeof(char));
//    str[0] = '1';
//    bstCreate(&bst, 8, str);
//    bstInsert(bst, 3, "123");
//    bstInsert(bst, 13, "1234");
//    bstInsert(bst, 11, "12543");
//    bstInsert(bst, 15, "1223");
//    bstInsert(bst, 12, "12233243");
//
//    bstPrint(bst);

//    printf("%d\n", findWordCount(bst, "12543"));
//    char **strings;
//    findMinMaxWords(bst, &strings);
//    for (int x = 0; x < 2; x ++) {
//        printf("%s\n", strings[x]);
//    }

//    if ((data = fopen(filename, "r")) == NULL) {
//        return 1;
//    }
//    fclose(data);

//    int key = 4;
//    int low = 0;
//    int high = 5 - 1;
//    int isFound = 0;
//    int arr[] = {1, 3, 5, 7, 8};
//    while (low <= high) {
//        int mid = (low + high) >> 1;
//        int midVal = arr[mid];
//
//        if (midVal < key) {
//            low = mid + 1;
//        } else if (midVal > key) {
//            high = mid - 1;
//        } else {
//            printf("%d\n", mid);
//            break;
//        }
//        printf("\n\n%d\n", low);
//    }
    return 0;
}