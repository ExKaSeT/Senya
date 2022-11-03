#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


typedef struct {
    char *data;
    unsigned int len;
} String;

void freeAll(int count, ...);

void printString(String *string);

int createString(String **result, const char *string);

void clearString(String *string);

void freeString(String *string);

int compareStrings(String *str1, String *str2, int (*comparator)(char, char));

int copyString(String *string, String *destination);

int concatStrings(String **result, int count, ...);

int duplicateString(String **result, String *string);

int stdCompString(char a, char b);