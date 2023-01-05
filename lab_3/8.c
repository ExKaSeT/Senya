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

int bstInsert(BstNode *root, int key, char *string, BstNode **result) {
//    char *str = (char *) malloc(sizeof(char) * 20);
//    strcpy(str, string);
//    string = str;

    BstNode *node = (BstNode *) malloc(sizeof(BstNode));
    if (node == NULL)
        return 1;
    node->key = key;
    node->string = string;
    bstInsertNode(root, node);
    if (result != NULL)
        *result = node;
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

void bstExcludeNode(BstNode **bstRoot, BstNode **exclude) {
    BstNode *node = *exclude;
    if (*bstRoot == *exclude) {
        if (node->left == NULL && node->right == NULL) {
            return;
        } else if (node->left == NULL || node->right == NULL) {
            if (node->left == NULL) {
                *bstRoot = node->right;
            } else {
                *bstRoot = node->left;
            }
            (*bstRoot)->parent = NULL;
        } else {
            BstNode *successor = bstNext(node);
            int keyTmp = node->key;
            char *stringTmp = node->string;
            node->key = successor->key;
            node->string = successor->string;
            if (successor->parent->left == successor) {
                successor->parent->left = successor->right;
                if (successor->right != NULL)
                    successor->right->parent = successor->parent;
            } else {
                successor->parent->right = successor->right;
                if (successor->right != NULL)
                    successor->right->parent = successor->parent;
            }
            successor->left = node->left;
            successor->right = node->right;
            successor->parent = node->parent;
            successor->key = keyTmp;
            successor->string = stringTmp;
            *exclude = successor;
        }
        return;
    }
    BstNode *p = (*exclude)->parent;
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
        int keyTmp = node->key;
        char *stringTmp = node->string;
        node->key = successor->key;
        node->string = successor->string;
        if (successor->parent->left == successor) {
            successor->parent->left = successor->right;
            if (successor->right != NULL)
                successor->right->parent = successor->parent;
        } else {
            successor->parent->right = successor->right;
            if (successor->right != NULL)
                successor->right->parent = successor->parent;
        }
        successor->left = node->left;
        successor->right = node->right;
        successor->parent = node->parent;
        successor->key = keyTmp;
        successor->string = stringTmp;
        *exclude = successor;
    }
}

void bstDestroy(BstNode *root) {
    if (root == NULL)
        return;
    if (root->left != NULL)
        bstDestroy(root->left);
    if (root->right != NULL)
        bstDestroy(root->right);
    free(root->string);
    free(root);
}

void findWordCountRec(BstNode *root, char *string, int *count) {
    if (*count != 0)
        return;
    if (root != NULL) {
        if (strcmp(string, root->string) == 0) {
            *count = root->key;
            return;
        }
        findWordCountRec(root->left, string, count);
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
        if (stringsCompare(root->string, strings[0]) < 0)
            strings[0] = root->string;
        if (stringsCompare(root->string, strings[1]) > 0)
            strings[1] = root->string;
        findMinMaxWordsRec(root->left, strings);
        findMinMaxWordsRec(root->right, strings);
    }
}

int findMinMaxWords(BstNode *root, char ***result) {
    char **res = (char **) malloc(sizeof(char *) * 2);
    if (res == NULL)
        return 1;
    res[0] = root->string;
    res[1] = res[0];
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

void findDepthRec(BstNode *node, int count, int *maxCount) {
    if (node == NULL)
        return;
    count++;
    if (node->left == NULL && node->right == NULL) {
        if (count > *maxCount)
            *maxCount = count;
        return;
    }
    findDepthRec(node->left, count, maxCount);
    findDepthRec(node->right, count, maxCount);
}

int findDepth(BstNode *root) {
    if (root == NULL)
        return 0;
    int depth = 0;
    findDepthRec(root, 0, &depth);
    return depth;
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
            if (node->link != NULL && link != NULL)
                return 2;
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
            if (low != node->len) {
                memmove(node->kids + low + 1, node->kids + low, sizeof(node->kids) * (node->len - low));
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

int insert(TrieNode *trie, BstNode *bst, char *string) {
    BstNode *bstNode;
    int statusCode = bstInsert(bst, 1, string, &bstNode);
    if (statusCode != 0) {
        return statusCode;
    }
    statusCode = trieInsert(trie, string, bstNode);
    if (statusCode != 0) {
        return statusCode;
    }
    return 0;
}

int trieIsContain(TrieNode *trie, char *string) {
    return trieSearch(trie, string) == NULL ? 0 : 1;
}

int isLexemeSymbol(char c) {
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
        return 1;
    return 0;
}

int processFile(BstNode **result, char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return 10;
    }
    TrieNode *trie;
    BstNode *bst = NULL;
    int bstLen = 0;
    int statusCode = trieCreate(&trie);
    if (statusCode != 0) {
        fclose(file);
        return statusCode;
    }
    int stringCapacity = 1;
    int len = 0;
    char *string = (char *) malloc(sizeof(char) * stringCapacity);
    if (string == NULL)
        return 1;
    int isBreak = 0;
    while (1) {
        char c = getc(file);
        if (c == EOF) {
            if (len == 0)
                break;
            isBreak = 1;
        }
        if (len != 0 && (!isLexemeSymbol(c) || isBreak)) {
            char *newString = (char *) realloc(string, sizeof(char) * (len + 1));
            if (newString == NULL) {
                free(string);
                trieDestroy(trie);
                if (bst != NULL)
                    bstDestroy(bst);
                fclose(file);
                return 1;
            }
            string = newString;
            string[len] = '\0';
            TrieNode *search = trieSearch(trie, string);
            if (search != NULL) {
                if (bstLen > 1) {
                    bstExcludeNode(&bst, &(search->link));
                    search->link->key++;
                    bstInsertNode(bst, search->link);
                } else {
                    search->link->key++;
                }
                free(string);
            } else {
                BstNode *bstNode;
                if (bstLen != 0) {
                    statusCode = bstInsert(bst, 1, string, &bstNode);
                } else {
                    statusCode = bstCreate(&bstNode, 1, string);
                    bst = bstNode;
                }
                if (statusCode != 0) {
                    free(string);
                    trieDestroy(trie);
                    if (bst != NULL)
                        bstDestroy(bst);
                    fclose(file);
                    return statusCode;
                }
                bstLen++;
                statusCode = trieInsert(trie, string, bstNode);
                if (statusCode != 0) {
                    trieDestroy(trie);
                    if (bst != NULL)
                        bstDestroy(bst);
                    fclose(file);
                    return statusCode;
                }
            }
            if (isBreak)
                break;
            stringCapacity = 1;
            len = 0;
            string = (char *) malloc(sizeof(char) * stringCapacity);
            if (string == NULL) {
                trieDestroy(trie);
                if (bst != NULL)
                    bstDestroy(bst);
                fclose(file);
                return 1;
            }
            continue;
        } else if (!isLexemeSymbol(c)) {
            continue;
        }
        if (len == stringCapacity) {
            stringCapacity *= 2;
            char *newString = (char *) realloc(string, sizeof(char) * stringCapacity);
            if (newString == NULL) {
                free(string);
                trieDestroy(trie);
                if (bst != NULL)
                    bstDestroy(bst);
                fclose(file);
                return 1;
            }
            string = newString;
        }
        string[len] = c;
        len++;
    }
    fclose(file);
//    triePrint(trie);
    trieDestroy(trie);
    if (bstLen == 0) {
        return 5;
    }
    *result = bst;
//    bstPrint(bst);
    return 0;
}

void bstSaveToFileRec(BstNode *node, FILE *file) {
    if (node == NULL)
        return;
    char *ptr = (char *) &node->key;
    for (int x = 0; x < sizeof(int); x++) {
        putc(*ptr++, file);
    }
    int stringSize = node->string == NULL ? 0 : strlen(node->string);
    ptr = (char *) &stringSize;
    for (int x = 0; x < sizeof(int); x++) {
        putc(*ptr++, file);
    }
    for (int x = 0; x < stringSize; x++) {
        putc(node->string[x], file);
    }
    bstSaveToFileRec(node->left, file);
    bstSaveToFileRec(node->right, file);
}

int bstSaveToFile(BstNode *root, char *filename) {
    if (root == NULL)
        return 2;
    FILE *file = fopen(filename, "w");
    if (file == NULL)
        return 10;
    bstSaveToFileRec(root, file);
    fclose(file);
    return 0;
}

int bstLoadFromFile(BstNode **result, char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return 10;
    BstNode *bst = NULL;
    int key = 0;
    int stringLen = 0;
    char c;
    while ((c = getc(file)) != EOF) {
        ungetc(c, file);
        char *ptr = (char *) &key;
        for (int x = 0; x < sizeof(int); x++) {
            c = getc(file);
            if (c == EOF) {
                fclose(file);
                bstDestroy(bst);
                return 10;
            }
            *ptr = c;
            ptr++;
        }
        ptr = (char *) &stringLen;
        for (int x = 0; x < sizeof(int); x++) {
            c = getc(file);
            if (c == EOF) {
                fclose(file);
                bstDestroy(bst);
                return 10;
            }
            *ptr = c;
            ptr++;
        }
        char *string = (char *) malloc(sizeof(char) * (stringLen + 1));
        if (string == NULL) {
            fclose(file);
            bstDestroy(bst);
            return 1;
        }
        string[stringLen] = '\n';
        ptr = string;
        for (int x = 0; x < stringLen; x++) {
            c = getc(file);
            if (c == EOF) {
                fclose(file);
                bstDestroy(bst);
                free(string);
                return 10;
            }
            *ptr = c;
            ptr++;
        }
        if (stringLen == 0) {
            free(string);
            string = NULL;
        }
        if (bst == NULL) {
            int statusCode = bstCreate(&bst, key, string);
            if (statusCode != 0) {
                fclose(file);
                bstDestroy(bst);
                free(string);
                return statusCode;
            }
        } else {
            int statusCode = bstInsert(bst, key, string, NULL);
            if (statusCode != 0) {
                fclose(file);
                bstDestroy(bst);
                free(string);
                return statusCode;
            }
        }
    }
    fclose(file);
    *result = bst;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        return 69;

    char *filename = argv[1];
    BstNode *bst;
    printf("%d func\n", bstLoadFromFile(&bst, "kek.txt"));
    bstPrint(bst);
//    printf("%d func\n", processFile(&bst, filename));
//    printf("%d func\n", bstSaveToFile(bst, "kek.txt"));

    return 0;
}