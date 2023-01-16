#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>
#include <windows.h>


typedef struct {
    unsigned value;
    char *name;
} MemoryCell;

typedef MemoryCell T;
typedef struct {
    T **list;
    int len;
} List;

typedef struct {
    List *ops;
    int saveResults; // 1 - left=; 0 - right=;
    int locationOfOp; // 0 - op(); 1 - ()op; 2 - (op);
    char *equalAlias; // first char alias to '=' must be uniq
    int inputBase;
    int outputBase;
} Settings;

int listCreate(List **list) {
    *list = (List *) malloc(sizeof(List));
    if ((*list) == NULL) {
        return 1;
    }
    (*list)->len = 0;
    return 0;
}

void listPrint(List *list) {
    printf("------------------------------\n");
    for (int x = 0; x < list->len; x++) {
        printf("%s = %d\n", list->list[x]->name, list->list[x]->value);
    }
    printf("------------------------------\n");
}

int listAdd(List *list, char *name, unsigned value, T **result, int isMakeNameCopy) {
    if (list == NULL || name == NULL) {
        return 2;
    }
    char *key = name;
    int low = 0;
    int high = list->len - 1;
    int isFound = 0;
    int mid;

    while (low <= high) {
        mid = (low + high) >> 1;
        char *midVal = list->list[mid]->name;

        int cmp = strcmp(midVal, key);
        if (cmp < 0) {
            low = mid + 1;
        } else if (cmp > 0) {
            high = mid - 1;
        } else {
            isFound = 1;
            break;
        }
    }
    if (isFound)
        return 2;

    T *new = (T *) malloc(sizeof(T));
    if (new == NULL) {
        return 1;
    }
    if (isMakeNameCopy) {
        char *nameCopy = (char *) malloc(sizeof(char) * (strlen(name) + 1));
        if (nameCopy == NULL) {
            free(new);
            return 1;
        }
        strcpy(nameCopy, name);
        name = nameCopy;
    }
    new->name = name;
    new->value = value;

    T **newList;
    if (list->len == 0) {
        newList = (T **) malloc(sizeof(T *));
    } else {
        newList = (T **) realloc(list->list, sizeof(T *) * (list->len + 1));
    }
    if (newList == NULL) {
        free(new);
        return 1;
    }
    list->list = newList;
    // low - index where insert new
    if (low != list->len) {
        memmove(list->list + low + 1, list->list + low, sizeof(list->list) * (list->len - low));
    }
    list->list[low] = new;
    list->len++;
    if (result != NULL)
        *result = new;
    return 0;
}

int listSearchIndex(List *list, char *name) {
    char *key = name;
    int low = 0;
    int high = list->len - 1;
    int isFound = 0;
    int mid;

    while (low <= high) {
        mid = (low + high) >> 1;
        char *midVal = list->list[mid]->name;

        int cmp = strcmp(midVal, key);
        if (cmp < 0) {
            low = mid + 1;
        } else if (cmp > 0) {
            high = mid - 1;
        } else {
            isFound = 1;
            break;
        }
    }
    if (isFound)
        return mid;
    return -1;
}

T *listSearch(List *list, char *name) {
    int index = listSearchIndex(list, name);
    if (index >= 0)
        return list->list[index];
    return NULL;
}

void listDestroy(List *list) {
    if (list == NULL)
        return;
    if (list->len != 0) {
        for (int x = 0; x < list->len; x++) {
            free(list->list[x]->name);
            free(list->list[x]);
        }
        free(list->list);
    }
    free(list);
}

/*
 * metSeparator may be '\0'
 * 2 : empty result
 * 1 : alloc error
 */
int readStringString(const char *source, char **result, const char *separators, char *metSeparator) {
    int resCapacity = 1;
    char *res = (char *) malloc(sizeof(char) * resCapacity);
    if (res == NULL)
        return 1;
    char c;
    int count = 0;
    while (1) {
        c = source[count];
        if (c == '\0') {
            if (metSeparator != NULL)
                *metSeparator = c;
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
    return 0;
}

/*
 * 2 : empty result
 * 1 : alloc error
 * -1 : EOF && NOT empty result
 * -2 : EOF && empty result
 */
int readStringFile(FILE *file, char **result, const char *separators, char *metSeparator) {
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

int stringRemoveChars(char **result, const char *string, const char *charsToRemove) {
    int resCapacity = 2;
    char *res = (char *) malloc(sizeof(char) * resCapacity);
    if (res == NULL)
        return 1;
    int count = 0;
    int index = 0;
    while (string[index] != '\0') {
        int x = 0;
        char c = string[index];
        int isSkip = 0;
        while (charsToRemove[x] != '\0') {
            if (c == charsToRemove[x]) {
                isSkip = 1;
                break;
            }
            x++;
        }
        index++;
        if (isSkip)
            continue;
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
    return 0;
}

int isDigitString(const char *string) {
    int x = 0;
    if (string[x] == '\0')
        return 0;
    while (string[x] != '\0') {
        if (string[x] < '0' || string[x] > '9')
            return 0;
        x++;
    }
    return 1;
}

int parseInt(const char *string) { // correct number in str
    int x = 0;
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
    return num;
}

int nextLine(FILE *file) {
    char c;
    while (1) {
        c = fgetc(file);
        if (c == EOF) {
            return -1;
        } else if (c == '\n') {
            return 0;
        }
    }
}

int isUnaryOperation(char operation) {
    if (operation == 'i' || operation == 'o')
        return 1;
    return 0;
}

int getStdListOps(List **result) {
    int opsCount = 10;
    char *opsNames[10] = {"add", "mult", "sub", "pow", "div", "rem", "xor", "input", "output", "="};
    char signs[] = {'+', '*', '-', 'p', '/', '%', '^', 'i', 'o', '='};
    List *ops;
    int statusCode = listCreate(&ops);
    if (statusCode != 0)
        return statusCode;
    for (int x = 0; x < opsCount; x++) {
        statusCode = listAdd(ops, opsNames[x], signs[x], NULL, 1);
        if (statusCode != 0) {
            listDestroy(ops);
            return statusCode;
        }
    }
    *result = ops;
    return 0;
}

int skipSettingsComment(FILE *file, char firstChar) {
    if (firstChar == '#') {
        return nextLine(file);
    } else if (firstChar == '[') {
        int bracketsCount = 1;
        while (bracketsCount) {
            char c = fgetc(file);
            if (c == EOF) {
                return -1;
            } else if (c == ']') {
                bracketsCount--;
            } else if (c == '[') {
                bracketsCount++;
            }
        }
        return nextLine(file);
    } else {
        return 2;
    }
}

int isCorrectEqualAlias(const char *string) {
    int x = 0;
    while (string[x] != '\0') {
        if (
                (string[x] >= '0' && string[x] <= '9') || (string[x] >= 'a' && string[x] <= 'z') ||
                (string[x] >= 'A' && string[x] <= 'Z') || string[x] == '_' || string[x] == ',' ||
                string[x] == '(' || string[x] == ')' || string[x] == '[' || string[x] == ']' ||
                string[x] == '#' || string[x] == ';' || string[x] == '\n' || string[x] == '\t' ||
                string[x] == ' '
                ) {
            return 0;
        }
        x++;
    }
    return 1;
}

int isCorrectAlias(const char *string, const char *equalAlias) {
    int x = 0;
    while (string[x] != '\0') {
        if (
                string[x] == ',' || string[x] == '(' || string[x] == ')' || string[x] == '[' ||
                string[x] == ']' || string[x] == '#' || string[x] == ';' || string[x] == equalAlias[0] ||
                string[x] == '\n' || string[x] == '\t' || string[x] == ' '
                ) {
            return 0;
        }
        x++;
    }
    return 1;
}

int stringCountSubstring(const char *string, const char *substring) {
    int count = 0;
    int x = 0;
    while (string[x] != '\0') {
        if (string[x] == substring[0]) {
            int y = 1;
            int tmpX = x + 1;
            while (substring[y] != '\0') {
                if (string[tmpX] == '\0' || string[tmpX] != substring[y])
                    break;
                tmpX += 1;
                y += 1;
            }
            if (substring[y] == '\0')
                count++;
        }
        x++;
    }
    return count;
}

int findIndexSubstring(const char *string, const char *substring) {
    int x = 0;
    while (string[x] != '\0') {
        if (string[x] == substring[0]) {
            int y = 1;
            int tmpX = x + 1;
            while (substring[y] != '\0') {
                if (string[tmpX] == '\0' || string[tmpX] != substring[y])
                    break;
                tmpX += 1;
                y += 1;
            }
            if (substring[y] == '\0')
                return x;
        }
        x++;
    }
    return -1;
}

int readSettingsFile(Settings **result, char *filename) {
    if (filename == NULL)
        return 2;
    Settings *settings = (Settings *) malloc(sizeof(Settings));
    if (settings == NULL) {
        return 1;
    }
    settings->saveResults = 1;
    settings->locationOfOp = 0;
    List *stdOps;
    int statusCode = getStdListOps(&stdOps);
    if (statusCode != 0) {
        free(settings);
        return statusCode;
    }
    char **newOpsNames = (char **) malloc(sizeof(char *) * stdOps->len);
    if (newOpsNames == NULL) {
        free(settings);
        listDestroy(stdOps);
        return 1;
    }
    for (int x = 0; x < stdOps->len; x++) {
        newOpsNames[x] = NULL;
    }
    FILE *data;
    if ((data = fopen(filename, "r")) == NULL) {
        free(settings);
        free(newOpsNames);
        listDestroy(stdOps);
        return 10;
    }
    char metSeparator;
    char *string;
    while (statusCode >= 0) {
        statusCode = readStringFile(data, &string, " #[]\n", &metSeparator);
        if (statusCode == 1) {
            fclose(data);
            free(settings);
            for (int x = 0; x < stdOps->len; x++)
                free(newOpsNames[x]);
            free(newOpsNames);
            listDestroy(stdOps);
            return statusCode;
        }
        if (metSeparator == ']') {
            fclose(data);
            free(settings);
            for (int x = 0; x < stdOps->len; x++)
                free(newOpsNames[x]);
            free(newOpsNames);
            listDestroy(stdOps);
            return 2;
        } else if (metSeparator == '[' || metSeparator == '#') {
            statusCode = skipSettingsComment(data, metSeparator);
            continue;
        }
        if (statusCode == 2 || statusCode == -2) { // empty string || empty string + EOF
            continue;
        }
        int skipLine = 0;
        if (strcmp(string, "left=") == 0) {
            settings->saveResults = 1;
            skipLine = 1;
        } else if (strcmp(string, "right=") == 0) {
            settings->saveResults = 0;
            skipLine = 1;
        } else if (strcmp(string, "op()") == 0) {
            settings->locationOfOp = 0;
            skipLine = 1;
        } else if (strcmp(string, "()op") == 0) {
            settings->locationOfOp = 1;
            skipLine = 1;
        } else if (strcmp(string, "(op)") == 0) {
            settings->locationOfOp = 2;
            skipLine = 1;
        }
        if (skipLine) {
            if (metSeparator == '\n') {
                free(string);
                continue;
            }
            free(string);
            nextLine(data);
            continue;
        } else if (metSeparator == '\n') {
            fclose(data);
            free(settings);
            for (int x = 0; x < stdOps->len; x++)
                free(newOpsNames[x]);
            free(newOpsNames);
            listDestroy(stdOps);
            return 2;
        }
        if (statusCode != 0) {
            fclose(data);
            free(string);
            free(settings);
            for (int x = 0; x < stdOps->len; x++)
                free(newOpsNames[x]);
            free(newOpsNames);
            listDestroy(stdOps);
            return 2;
        }
        int opIndex = listSearchIndex(stdOps, string);
        free(string);
        if (opIndex == -1) {
            fclose(data);
            free(settings);
            for (int x = 0; x < stdOps->len; x++)
                free(newOpsNames[x]);
            free(newOpsNames);
            listDestroy(stdOps);
            return 2;
        }
        statusCode = readStringFile(data, &string, " \t\n", &metSeparator);
        if (statusCode != 0 && statusCode != -1) {
            fclose(data);
            free(settings);
            for (int x = 0; x < stdOps->len; x++)
                free(newOpsNames[x]);
            free(newOpsNames);
            listDestroy(stdOps);
            return 2;
        }
        if (newOpsNames[opIndex] != NULL)
            free(newOpsNames[opIndex]);
        newOpsNames[opIndex] = string;
        if (metSeparator != '\n')
            statusCode = nextLine(data);
    }
    fclose(data);
    for (int x = 0; x < stdOps->len; x++) {
        char *name1 = newOpsNames[x] == NULL ? stdOps->list[x]->name : newOpsNames[x];
        for (int y = 0; y < stdOps->len; y++) {
            if (x == y)
                continue;
            char *name2 = newOpsNames[y] == NULL ? stdOps->list[y]->name : newOpsNames[y];
            if (strcmp(name1, name2) == 0) {
                free(settings);
                for (int x = 0; x < stdOps->len; x++)
                    free(newOpsNames[x]);
                free(newOpsNames);
                listDestroy(stdOps);
                return 99;
            }
        }
    }
    int equalIndex = listSearchIndex(stdOps, "=");
    char *equalOpName;
    if (newOpsNames[equalIndex] != NULL) {
        equalOpName = newOpsNames[equalIndex];
    } else {
        equalOpName = stdOps->list[equalIndex]->name;
    }
    if (!isCorrectEqualAlias(equalOpName)) {
        free(settings);
        for (int x = 0; x < stdOps->len; x++)
            free(newOpsNames[x]);
        free(newOpsNames);
        listDestroy(stdOps);
        return 99;
    }
    for (int x = 0; x < stdOps->len; x++) {
        if (newOpsNames[x] == NULL)
            continue;
        if (x != equalIndex && !isCorrectAlias(newOpsNames[x], equalOpName)) {
            free(settings);
            for (int x = 0; x < stdOps->len; x++)
                free(newOpsNames[x]);
            free(newOpsNames);
            listDestroy(stdOps);
            return 99;
        }
        free(stdOps->list[x]->name);
        stdOps->list[x]->name = newOpsNames[x];
    }
    free(newOpsNames);

    List *ops;
    statusCode = listCreate(&ops);
    if (statusCode != 0) {
        listDestroy(stdOps);
        free(settings);
        return statusCode;
    }
    for (int x = 0; x < stdOps->len; x++) {
        if (x == equalIndex) {
            T *op;
            statusCode = listAdd(ops, stdOps->list[x]->name, stdOps->list[x]->value, &op, 1);
            settings->equalAlias = op->name;
        } else {
            statusCode = listAdd(ops, stdOps->list[x]->name, stdOps->list[x]->value, NULL, 1);
        }
        if (statusCode != 0) {
            listDestroy(stdOps);
            listDestroy(ops);
            free(settings);
        }
    }
    listDestroy(stdOps);

    settings->ops = ops;
    *result = settings;
    return 0;
}

int skipInstructionsComment(FILE *file, char firstChar) {
    if (firstChar == '#') {
        return nextLine(file);
    } else if (firstChar == '[') {
        int bracketsCount = 1;
        while (bracketsCount) {
            char c = fgetc(file);
            if (c == EOF) {
                return -1;
            } else if (c == ']') {
                bracketsCount--;
            } else if (c == '[') {
                bracketsCount++;
            }
        }
    } else {
        return 2;
    }
    return 0;
}

int skipSepSymbols(FILE *file) {
    while (1) {
        char c = fgetc(file);
        if (c == EOF) {
            return -1;
        } else if (!(c == ' ' || c == '\n' || c == '\t')) {
            ungetc(c, file);
            return 0;
        }
    }
}

int isCorrectVarChar(char c) {
    return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') || c == '_');
}

int isCorrectVarName(const char *string) {
    if (string[0] >= '0' && string[0] <= '9')
        return 0;
    int x = 0;
    while (string[x] != '\0') {
        if (!isCorrectVarChar(string[x])) {
            return 0;
        }
        x++;
    }
    return 1;
}

// return index of string
int skipSepSymbolsString(const char *string, unsigned startIndex) {
    while (string[startIndex] == ' ' || string[startIndex] == '\t' || string[startIndex] == '\n') {
        startIndex++;
    }
    return startIndex;
}

// string: "op(var)"
int parseBinaryOperation(Settings *settings, char *string,
                         char *getOperation, char **getVar1, char **getVar2) {
    char *operation, *var1, *var2;
    char metSeparator = ' ';
    if (settings->locationOfOp == 0) { // op()
        int statusCode = readStringString(string, &operation, "(", &metSeparator);
        if (statusCode != 0)
            return statusCode;
        if (metSeparator == '\0') {
            free(operation);
            return 2;
        }
        int index = strlen(operation) + 1;
        if (string[index] == '\0') {
            free(operation);
            return 2;
        }
        statusCode = readStringString(string + index, &var1, ",", &metSeparator);
        if (statusCode != 0) {
            free(operation);
            return 2;
        }
        if (metSeparator == '\0') {
            free(var1);
            free(operation);
            return 2;
        }
        index += strlen(var1) + 1;
        if (string[index] == '\0') {
            free(var1);
            free(operation);
            return 2;
        }
        statusCode = readStringString(string + index, &var2, ")", &metSeparator);
        if (statusCode != 0) {
            free(var1);
            free(operation);
            return 2;
        }
        if (metSeparator == '\0' || string[index + 1] != '\0') {
            free(var2);
            free(var1);
            free(operation);
            return 2;
        }
    } else if (settings->locationOfOp == 1) { // ()op
        if (string[0] != '(') {
            return 2;
        }
        int index = 1;
        if (string[index] == '\0') {
            return 2;
        }
        int statusCode = readStringString(string + index, &var1, ",", &metSeparator);
        if (statusCode != 0)
            return 2;
        if (metSeparator == '\0') {
            free(var1);
            return 2;
        }
        index += strlen(var1) + 1;
        if (string[index] == '\0') {
            free(var1);
            return 2;
        }
        statusCode = readStringString(string + index, &var2, ")", &metSeparator);
        if (statusCode != 0) {
            free(var1);
            return 2;
        }
        if (metSeparator == '\0') {
            free(var2);
            free(var1);
            return 2;
        }
        index += strlen(var2) + 1;
        if (string[index] == '\0') {
            free(var2);
            free(var1);
            return 2;
        }
        statusCode = readStringString(string + index, &operation, "\0", &metSeparator);
        if (statusCode != 0) {
            free(var2);
            free(var1);
            return statusCode;
        }
    } else { // var1#op#var2
        int statusCode = readStringString(string, &var1, "#", &metSeparator);
        if (statusCode != 0)
            return 2;
        if (metSeparator == '\0') {
            free(var1);
            return 2;
        }
        int index = strlen(var1) + 1;
        if (string[index] == '\0') {
            free(var1);
            return 2;
        }
        statusCode = readStringString(string + index, &operation, "#", &metSeparator);
        if (statusCode != 0) {
            free(var1);
            return statusCode;
        }
        if (metSeparator == '\0') {
            free(var1);
            free(operation);
            return 2;
        }
        index += strlen(operation) + 1;
        if (string[index] == '\0') {
            free(var1);
            free(operation);
            return 2;
        }
        statusCode = readStringString(string + index, &var2, "\0", NULL);
        if (statusCode != 0) {
            free(operation);
            free(var1);
            return statusCode;
        }
    }
    T *op = listSearch(settings->ops, operation);
    if (op == NULL || isUnaryOperation(op->value) || !isCorrectVarName(var1) || !isCorrectVarName(var2)) {
        free(operation);
        free(var1);
        free(var2);
        return 2;
    }
    *getOperation = (char) op->value;
    *getVar1 = var1;
    *getVar2 = var2;
    return 0;
}

// string: "op(var1,var2) "
int parseUnaryOp(Settings *settings, char *string, char *getOperation, char **getArgName) {
    if (settings == NULL)
        return 2;
    char *operation;
    char *argName;
    char metSeparator = ' ';
    if (settings->locationOfOp == 0 || settings->locationOfOp == 2) { // op(var)
        int statusCode = readStringString(string, &operation, "(", &metSeparator);
        if (statusCode != 0)
            return statusCode;
        if (metSeparator == '\0') {
            return 2;
        }
        int index = strlen(operation) + 1;
        if (string[index] == '\0') {
            free(operation);
            return 2;
        }
        statusCode = readStringString(string + index, &argName, ")", &metSeparator);
        if (statusCode != 0) {
            free(operation);
            return statusCode;
        }
        index += strlen(argName) + 1;
        if (string[index] != '\0') {
            free(operation);
            free(argName);
            return 2;
        }
    } else { // (var)op
        if (string[0] != '(')
            return 2;
        int index = 1;
        if (string[index] == '\0')
            return 2;
        int statusCode = readStringString(string + index, &argName, ")", &metSeparator);
        if (statusCode != 0)
            return statusCode;
        if (metSeparator == '\0') {
            free(argName);
            return 2;
        }
        index += strlen(argName) + 1;
        if (string[index] == '\0') {
            free(argName);
            return 2;
        }
        statusCode = readStringString(string + index, &operation, "\0", &metSeparator);
        if (statusCode != 0) {
            free(argName);
            return statusCode;
        }
        if (metSeparator != '\0') {
            free(argName);
            free(operation);
            return 2;
        }
    }
    T *op = listSearch(settings->ops, operation);
    if (op == NULL || !isUnaryOperation(op->value) || !isCorrectVarName(argName)) {
        free(operation);
        free(argName);
        return 2;
    }
    *getOperation = (char) op->value;
    *getArgName = argName;
    return 0;
}

int inputOp(Settings *settings, unsigned int *result) {
    int base = settings->inputBase;
    if (base < 2 || base > 36)
        return 2;
    char number[100];
    for (int x = 0; x < 100; x++)
        number[x] = '0';
    number[99] = '\0';
    scanf("%s", number);
    if (number[99] != '\0')
        return 99;
    int zeroCount = 0;
    for (int x = 0; x < 99; x++) {
        if (number[x] == '0')
            zeroCount++;
        else
            break;
    }
    unsigned intNumber = 0;
    unsigned tmp = 1;
    int index = strlen(number) - 1;
    while (index != zeroCount - 1) {
        if ((number[index] < '0' || number[index] > '9') && (number[index] < 'A' || number[index] > 'Z') && (number[index] < 'a' || number[index] > 'z'))
            return 2;
        int num = isdigit(number[index]) ? (number[index] - '0') : (tolower(number[index]) - 'a' + 10);
        if (num + 1 > base)
            return 2;
        if (num > (UINT_MAX - intNumber) / tmp)
            return 100;
        intNumber += num * tmp;
        if (UINT_MAX / base < tmp)
            return 100;
        tmp *= base;
        index--;
    }
    *result = intNumber;
    return 0;
}

char reVal(int num) {
    if (num >= 0 && num <= 9)
        return (char) (num + '0');
    else
        return (char) (num - 10 + 'A');
}

int outputOp(Settings *settings, unsigned value) {
    unsigned base = settings->outputBase;
    if (base < 2 || base > 36)
        return 2;
    int resCapacity = 1;
    char *res = (char *) malloc(sizeof(char) * resCapacity);
    if (res == NULL)
        return 1;
    int index = 0;
    if (value == 0)
        putchar('0');
    while (value > 0) {
        if (index == resCapacity) {
            resCapacity *= 2;
            char *newRes = (char *) realloc(res, sizeof(char) * resCapacity);
            if (newRes == NULL) {
                free(res);
                return 1;
            }
            res = newRes;
        }
        res[index++] = reVal(value % base);
        value /= base;
    }
    index--;
    while (index >= 0) {
        putchar(res[index]);
        index--;
    }
    putchar('\n');
    free(res);
    return 0;
}

int isSeparator(char c) {
    return (c == ' ' || c == '\t' || c == '\n');
}

int isCorrectOpBetween(const char *string) { // "var1 op var2" - correct
    int x = 0;
    while (string[x] != ' ' && string[x] != '\0') {
        if (!isCorrectVarChar(string[x]))
            return 0;
        x++;
    }
    if (string[x] == '\0')
        return 0;
    x++;
    if (isSeparator(string[x]))
        return 0;
    x++;
    while (string[x] != ' ' && string[x] != '\0') {
        if (isSeparator(string[x]))
            return 0;
        x++;
    }
    x++;
    if (isSeparator(string[x]))
        return 0;
    while (string[x] != '\0') {
        if (!isCorrectVarChar(string[x]))
            return 0;
        x++;
    }
    return 1;
}

int processingInstructions(Settings *settings, char *filename) {
    if (settings == NULL || filename == NULL || settings->ops == NULL || settings->equalAlias == NULL)
        return 2;
    FILE *data;
    if ((data = fopen(filename, "r")) == NULL) {
        return 10;
    }
    List *vars;
    int statusCode = listCreate(&vars);
    if (statusCode != 0) {
        fclose(data);
        return statusCode;
    }
    char equalAliasFirstChar = settings->equalAlias[0];
    char metSeparator;
    char operation;
    char *string;
    while (statusCode >= 0) {
        statusCode = skipSepSymbols(data);
        if (statusCode < 0)
            continue;
        char c = fgetc(data);
        statusCode = skipInstructionsComment(data, c);
        if (statusCode == -1)
            continue;
        ungetc(c, data);
        statusCode = skipSepSymbols(data);
        if (statusCode < 0)
            continue;
        statusCode = readStringFile(data, &string, ";", NULL);
        if (statusCode != 0) {
            fclose(data);
            if (statusCode == -1)
                free(string);
            listDestroy(vars);
            return statusCode;
        }
        int equalCount = stringCountSubstring(string, settings->equalAlias);
        if (equalCount > 1) {
            fclose(data);
            free(string);
            listDestroy(vars);
            return 2;
        }
        if (equalCount) { // binary operation
            if (settings->locationOfOp == 2) { // var1 op var2
                if (settings->saveResults) { // left=
                    int opStartIndex = findIndexSubstring(string, settings->equalAlias) + strlen(settings->equalAlias);
                    while (isSeparator(string[opStartIndex]))
                        opStartIndex++;
                    if (!isCorrectOpBetween(string + opStartIndex)) {
                        fclose(data);
                        free(string);
                        listDestroy(vars);
                        return 2;
                    }
                    while (string[opStartIndex] != '\0') {
                        if (string[opStartIndex] == ' ')
                            string[opStartIndex] = '#';
                        opStartIndex++;
                    }
                } else {
                    int opEndIndex = findIndexSubstring(string, settings->equalAlias);
                    while (isSeparator(string[opEndIndex]))
                        opEndIndex--;
                    string[opEndIndex] = '\0';
                    if (!isCorrectOpBetween(string)) {
                        fclose(data);
                        free(string);
                        listDestroy(vars);
                        return 2;
                    }
                    int x = 0;
                    while (string[x] != '\0') {
                        if (string[x] == ' ')
                            string[x] = '#';
                        x++;
                    }
                    string[opEndIndex] = ' ';
                }
                // var1 op var2 -> var1#op#var2
            }
            char *newString;
            statusCode = stringRemoveChars(&newString, string, " \t\n");
            if (statusCode != 0) {
                fclose(data);
                free(string);
                listDestroy(vars);
                return statusCode;
            }
            free(string);
            string = newString;
            char *varName;
            char *arg1, *arg2;
            if (settings->saveResults) { // left=
                int indexOfEqualAlias = findIndexSubstring(string, settings->equalAlias);
                string[indexOfEqualAlias] = '\0';
                statusCode = readStringString(string, &varName, "\0", NULL);
                if (statusCode != 0) {
                    fclose(data);
                    free(string);
                    listDestroy(vars);
                    return statusCode;
                }
                string[indexOfEqualAlias] = equalAliasFirstChar;
                int index = strlen(varName) + strlen(settings->equalAlias);
                statusCode = parseBinaryOperation(settings, string + index, &operation, &arg1, &arg2);
                if (statusCode != 0) {
                    fclose(data);
                    free(string);
                    listDestroy(vars);
                    free(varName);
                    return statusCode;
                }
            } else { // =right
                int indexOfEqualAlias = findIndexSubstring(string, settings->equalAlias);
                string[indexOfEqualAlias] = '\0';
                statusCode = parseBinaryOperation(settings, string, &operation, &arg1, &arg2);
                if (statusCode != 0) {
                    fclose(data);
                    free(string);
                    listDestroy(vars);
                    return statusCode;
                }
                string[indexOfEqualAlias] = equalAliasFirstChar;
                int index = indexOfEqualAlias + strlen(settings->equalAlias);
                statusCode = readStringString(string + index, &varName, "\0", NULL);
                if (statusCode != 0) {
                    fclose(data);
                    free(string);
                    listDestroy(vars);
                    return statusCode;
                }
            }
            // все остальное проверяется в parseBinaryOperation()
            if (!isCorrectVarName(varName)) {
                fclose(data);
                free(string);
                listDestroy(vars);
                free(varName);
                free(arg1);
                free(arg2);
                return 2;
            }
            T *arg1Link = listSearch(vars, arg1);
            T *arg2Link = listSearch(vars, arg2);
            if (arg1Link == NULL || arg2Link == NULL) {
                fclose(data);
                free(string);
                listDestroy(vars);
                free(varName);
                return 2;
            }
            free(arg1);
            free(arg2);
            T *var = listSearch(vars, varName);
            if (var == NULL) {
                statusCode = listAdd(vars, varName, 0, &var, 1);
                if (statusCode != 0) {
                    fclose(data);
                    free(varName);
                    free(string);
                    listDestroy(vars);
                    return statusCode;
                }
            }
            free(varName);
            // char signs[] = {'+', '*', '-', 'p', '/', '%', '^', 'i', 'o', '='};
            switch (operation) {
                case '+': {
                    var->value = arg1Link->value + arg2Link->value;
                    break;
                }
                case '*': {
                    var->value = arg1Link->value * arg2Link->value;
                    break;
                }
                case '-': {
                    var->value = arg1Link->value - arg2Link->value;
                    break;
                }
                case 'p': {
                    var->value = (unsigned) pow(arg1Link->value, arg2Link->value);
                    break;
                }
                case '/': {
                    if (arg2Link->value == 0) {
                        fclose(data);
                        free(string);
                        listDestroy(vars);
                        return 10;
                    }
                    var->value = arg1Link->value / arg2Link->value;
                    break;
                }
                case '%': {
                    var->value = arg1Link->value % arg2Link->value;
                    break;
                }
                case '^': {
                    var->value = arg1Link->value ^ arg2Link->value;
                    break;
                }
                default: {
                    fclose(data);
                    free(string);
                    listDestroy(vars);
                    return 11;
                }
            }
        } else { // unary: input or output operations
            char *newString;
            statusCode = stringRemoveChars(&newString, string, " \t\n");
            if (statusCode != 0) {
                fclose(data);
                free(string);
                listDestroy(vars);
                return statusCode;
            }
            free(string);
            string = newString;
            char *argName;
            statusCode = parseUnaryOp(settings, string, &operation, &argName);
            if (statusCode != 0) {
                fclose(data);
                free(string);
                listDestroy(vars);
                return statusCode;
            }
            T *var = listSearch(vars, argName);
            if (var == NULL && operation == 'o') {
                fclose(data);
                free(argName);
                free(string);
                listDestroy(vars);
                return 2;
            }
            if (var == NULL) {
                statusCode = listAdd(vars, argName, 0, &var, 1);
                if (statusCode != 0) {
                    fclose(data);
                    free(argName);
                    free(string);
                    listDestroy(vars);
                    return statusCode;
                }
            }
            free(argName);
            switch (operation) {
                case 'i': {
                    unsigned int value;
                    statusCode = inputOp(settings, &value);
                    if (statusCode != 0) {
                        fclose(data);
                        free(string);
                        listDestroy(vars);
                        return statusCode;
                    }
                    var->value = value;
                    break;
                }
                case 'o': {
                    statusCode = outputOp(settings, var->value);
                    if (statusCode != 0) {
                        fclose(data);
                        free(string);
                        listDestroy(vars);
                        return statusCode;
                    }
                    break;
                }
                default: {
                    fclose(data);
                    free(string);
                    listDestroy(vars);
                    return 11;
                }
            }
        }
        free(string);
    }
    fclose(data);
    listDestroy(vars);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 5 || !isDigitString(argv[3]) || !isDigitString(argv[4]))
        return 69;

    int inputBase = parseInt(argv[3]);
    int outputBase = parseInt(argv[4]);
    if (inputBase < 2 || inputBase > 36 || outputBase < 2 || outputBase > 36)
        return 2;

    Settings *settings;
    int statusCode;
    if (strcmp(argv[1], ".") == 0) {
        statusCode = readSettingsFile(&settings, "settings.txt");
        if (statusCode != 0) {
            return statusCode;
        }
    } else {
        statusCode = readSettingsFile(&settings, argv[1]);
        if (statusCode != 0) {
            return statusCode;
        }
        CopyFile(argv[1], "settings.txt", FALSE);
    }

    settings->inputBase = inputBase;
    settings->outputBase = outputBase;

    statusCode = processingInstructions(settings, argv[2]);
    listDestroy(settings->ops);
    free(settings);
    if (statusCode != 0)
        return statusCode;
    return 0;
}