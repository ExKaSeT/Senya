#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


void freeAll(int count, ...) {
    va_list args;
    void *ptr;
    va_start(args, count);
    for (int x = 0; x < count; x++) {
        ptr = va_arg(args, void *);
        free(ptr);
    }
}

int isLetter(char c) {
    return (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'));
}

int readString(FILE *file, char **result, char *separators, int (*isCorrectChar)(char)) {
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
    if (isEOF)
        return -1;
    return 0;
}

int writePhrase(FILE *out, const char *phrase, int offsetSize) {
    if (offsetSize < 3)
        return 2;
    int y = 0;
    int index = 0;
    char key[26];
    int value[26]; // value for alignment
    int childCount[26];
    while (phrase[y] != '\0') {
        if (isLetter(phrase[y])) {
            index = phrase[y] - 'A';
            key[index] = phrase[y];
            value[index] = 0;
            childCount[index] = 0;
            int z = y + 1;
            int level = -1;
            while (phrase[z] != '\0') {
                char c = phrase[z];
                if (isLetter(c) && level == 0) {
                    childCount[index]++;
                }
                if (c == '(') {
                    level++;
                } else if (c == ')') {
                    level--;
                    if (level < 0)
                        break;
                } else if (c == ',') {
                    if (level < 0)
                        break;
                    value[index]++;
                }
                z++;
            }
        }
        y++;
    }

    int dotCount = 0;
    int linesSize = 1;
    int openedBrackets = 0;
    int x = 0;
    while (phrase[x] != '\0') {
        if (phrase[x] == ',') {
            dotCount++;
        } else if (phrase[x] == '(') {
            openedBrackets++;
            if (openedBrackets + 1 > linesSize)
                linesSize = openedBrackets + 1;
        } else if (phrase[x] == ')') {
            openedBrackets--;
        }
        x++;
    }
    char *parents = (char *) malloc(sizeof(char) * linesSize);
    int lineSize = offsetSize * dotCount + 2;
    char **lines = (char **) malloc(sizeof(char *) * linesSize);
    char **slashes = (char **) malloc(sizeof(char *) * linesSize);
    if (parents == NULL || lines == NULL || slashes == NULL) {
        freeAll(3, parents, lines, slashes);
        return 1;
    }
    for (int x = 0; x < linesSize; x++) {
        lines[x] = malloc(sizeof(char) * lineSize);
        lines[x][lineSize - 1] = '\0';
        slashes[x] = malloc(sizeof(char) * lineSize);
        slashes[x][lineSize - 1] = '\0';
        if (lines[x] == NULL || slashes[x] == NULL) {
            for (int y = 0; y <= x; y++) {
                freeAll(5, lines[y], slashes[y], lines, slashes, parents);
                return 1;
            }
        }
    }
    for (int x = 0; x < linesSize; x++) {
        for (int y = 0; y < lineSize - 1; y++) {
            lines[x][y] = ' ';
            slashes[x][y] = ' ';
        }
    }

    int offset = 0;
    int line = 0;
    int isFirst = 1;
    char lastLetter;
    x = 0;
    while (phrase[x] != '\0') {
        char c = phrase[x];
        if (isLetter(c)) {
            lastLetter = c;
            lines[line][offset + (value[c - 'A'] * offsetSize) / 2] = c;
            if (line != 0) {
                char parent = parents[line - 1];
                if (isFirst) {
                    if (childCount[parent - 'A'] == 1) {
                        slashes[line - 1][offset + (value[c - 'A'] * offsetSize) / 2] = '|';
                    } else {
                        slashes[line - 1][offset + 1 + (value[c - 'A'] * offsetSize) / 2] = '/';
                    }
                    childCount[parent - 'A']--;
                } else if (childCount[parent - 'A'] == 1) {
                    slashes[line - 1][offset - 1 + (value[c - 'A'] * offsetSize) / 2] = '\\';
                    childCount[parent - 'A']--;
                } else {
                    slashes[line - 1][offset + (value[c - 'A'] * offsetSize) / 2] = '|';
                    childCount[parent - 'A']--;
                }
            }
        } else if (c == '(') {
            parents[line] = lastLetter;
            isFirst = 1;
            line++;
        } else if (c == ',') {
            isFirst = 0;
            offset += offsetSize;
        } else if (c == ')') {
            line--;
        }
        x++;
    }

    fprintf(out, "%s\n\n", phrase);
    for (int x = 0; x < linesSize; x++) {
        fprintf(out, "%s\n", lines[x]);
        fprintf(out, "%s\n", slashes[x]);
        freeAll(2, lines[x], slashes[x]);
    }
    freeAll(3, lines, slashes, parents);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        return 69;

    FILE *data, *out;
    char *phrase;
    char *filename = argv[1];

    if ((data = fopen(filename, "r")) == NULL) {
        return 1;
    }
    if ((out = fopen("out.txt", "w")) == NULL) {
        return 2;
    }

    int statusCode = 0;
    int statusCode1 = 0;
    while (statusCode != -1) {
        statusCode = readString(data, &phrase, "\n", NULL);
        if (statusCode > 0) {
            fclose(data);
            fclose(out);
            return statusCode;
        }
        statusCode1 = writePhrase(out, phrase, 4);
        if (statusCode > 0) {
            fclose(data);
            fclose(out);
            free(phrase);
            return statusCode1;
        }
        free(phrase);
    }

    fclose(data);
    fclose(out);
    return 0;
}