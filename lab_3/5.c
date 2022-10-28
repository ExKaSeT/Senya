#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <errno.h>


typedef struct {
    unsigned int id;
    char *lastname;
    char *name;
    int course;
    char *group;
    int *marks;
} Student;

void freeAll(int count, ...) {
    va_list args;
    void *ptr;
    va_start(args, count);
    for (int x = 0; x < count; x++) {
        ptr = va_arg(args, void *);
        free(ptr);
    }
}

short isLetter(char c) {
    return (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'));
}

short isNum(char c) {
    return ('0' <= c && c <= '9');
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
    }
    return num;
}

short readStudent(FILE *file, Student **result) {
    short isLastStudent;
    char *idString, *lastname, *name, *courseString, *group, *marksString;
    int id, course;
    int *marks;
    short statusCode;
    statusCode = readString(file, &idString, "\t ", isNum);
    if (statusCode != 0) {
        return statusCode;
    }
    id = parseInt(idString);
    free(idString);
    if (id < 0)
        return 11;
    statusCode = readString(file, &lastname, "\t ", isLetter);
    if (statusCode != 0) {
        return statusCode;
    }
    statusCode = readString(file, &name, "\t ", isLetter);
    if (statusCode != 0) {
        free(lastname);
        return statusCode;
    }
    statusCode = readString(file, &courseString, "\t ", isNum);
    if (statusCode != 0) {
        freeAll(2, lastname, name);
        return statusCode;
    }
    course = parseInt(courseString);
    free(courseString);
    if (course < 1 || course > 4) {
        freeAll(2, lastname, name);
        return 12;
    }
    statusCode = readString(file, &group, "\t ", NULL);
    if (statusCode != 0) {
        freeAll(2, lastname, name);
        return statusCode;
    }
    statusCode = readString(file, &marksString, "\n", isNum);
    if (statusCode != 0) {
        if (statusCode == -1) { // EOF
            isLastStudent = 1;
        } else {
            freeAll(3, lastname, name, group);
            return statusCode;
        }
    }
    if (strlen(marksString) != 5) {
        freeAll(3, lastname, name, group);
        return 4;
    }
    marks = (int *) malloc(sizeof(int) * 5);
    if (marks == NULL) {
        freeAll(4, lastname, name, group, marksString);
        return 5;
    }
    for (int x = 0; x < 5; x++) {
        marks[x] = (marksString[x] - '0');
        if (marks[x] < 2 || marks[x] > 5) {
            freeAll(5, lastname, name, group, marksString, marks);
            return 13;
        }
    }
    free(marksString);
    Student *student = (Student *) malloc(sizeof(Student));
    if (student == NULL) {
        freeAll(3, lastname, name, group);
        return 6;
    }
    student->id = id;
    student->lastname = lastname;
    student->name = name;
    student->course = course;
    student->group = group;
    student->marks = marks;
    *result = student;
    if (isLastStudent)
        return -1; // EOF
    return 0;
}

void killStudents(Student **list, int len) {
    for (int x = 0; x < len; x++) {
        freeAll(4, list[x]->marks, list[x]->group, list[x]->lastname, list[x]->name);
        free(list[x]);
    }
}

short readStudents(FILE *file, Student ***result, int *len) {
    short statusCode = 0;
    int count = 0;
    int resCapacity = 1;
    Student *student;

    Student **res = (Student **) malloc(sizeof(Student) * resCapacity);
    if (res == NULL)
        return 10;

    while (statusCode != -1) { // -1 = EOF
        if (count == resCapacity) {
            resCapacity *= 2;
            Student **newRes = (Student **) realloc(res, sizeof(Student) * resCapacity);
            if (newRes == NULL) {
                killStudents(res, count);
                return 11;
            }
            res = newRes;
        }
        statusCode = readStudent(file, &student);
        if (statusCode != 0 && statusCode != -1) {
            free(res);
            return statusCode;
        }

        res[count] = student;
        count++;
    }
    if (resCapacity != count) {
        Student **newRes = (Student **) realloc(res, sizeof(Student) * count);
        if (newRes == NULL) {
            killStudents(res, count);
            return 11;
        }
        res = newRes;
    }

    *result = res;
    *len = count;
    return 0;
}

int averageMark(const int *marks) {
    int sumMarks = 0;
    for (int x = 0; x < 5; x++)
        sumMarks += marks[x];
    return ((int) round(sumMarks / 5.0));
}

void printStudents(Student **list, int len) {
    for (int x = 0; x < len; x++) {
        printf("\n------------------------------\n%s\n%s\ncourse: %d\ngroup: %s\naverage mark: %d\n------------------------------\n",
               list[x]->lastname, list[x]->name, list[x]->course, list[x]->group,
               averageMark(list[x]->marks));
    }
}

int studentsAverageMark(Student **list, int len) {
    int sumMarks = 0;
    for (int y = 0; y < len; y++) {
        for (int x = 0; x < 5; x++)
            sumMarks += list[y]->marks[x];
    }
    return ((int) round(sumMarks / (double) (5 * len)));
}

void swap(Student **a, Student **b) {
    Student *x = *a;
    *a = *b;
    *b = x;
}

int partition(Student **array, int low, int high, int (*compareTo)(Student *, Student *)) {
    Student *pivot = array[high];
    int i = (low - 1);
    for (int j = low; j < high; j++) {
        if (compareTo(array[j], pivot) != 1) {
            i++;
            swap(&array[i], &array[j]);
        }
    }
    swap(&array[i + 1], &array[high]);
    return (i + 1);
}

void quickSort(Student **array, int low, int high, int (*compareTo)(Student *, Student *)) {
    if (low < high) {
        int pi = partition(array, low, high, compareTo);
        quickSort(array, low, pi - 1, compareTo);
        quickSort(array, pi + 1, high, compareTo);
    }
}

void sortStudents(Student **list, int len, int (*compareTo)(Student *, Student *)) {
    quickSort(list, 0, len - 1, compareTo);
}

int compareByName(Student *s1, Student *s2) {
    return strcmp(s1->name, s2->name);
}

int compareByLastname(Student *s1, Student *s2) {
    return strcmp(s1->lastname, s2->lastname);
}

int compareByGroup(Student *s1, Student *s2) {
    return strcmp(s1->group, s2->group);
}

int compareById(Student *s1, Student *s2) {
    if (s1->id > s2->id)
        return 1;
    if (s1->id < s2->id)
        return -1;
    return 0;
}

short isMarksEqual(int *marks1, int *marks2) {
    for (int x = 0; x < 5; x++) {
        if (marks1[x] != marks2[x])
            return 0;
    }
    return 1;
}

short findStudents(Student **list, int len, Student ***result, int *resLen,
                   unsigned int *id, char *lastname, char *name, int *course,
                   char *group, int *marks) {
    int count = 0;
    int resCapacity = 1;
    Student *student;

    Student **res = (Student **) malloc(sizeof(Student *) * resCapacity);
    if (res == NULL)
        return 1;

    for (int x = 0; x < len; x++) {
        student = list[x];
        if (count == resCapacity) {
            resCapacity *= 2;
            Student **newRes = (Student **) realloc(res, sizeof(Student *) * resCapacity);
            if (newRes == NULL) {
                free(res);
                return 2;
            }
            res = newRes;
        }

        if (lastname != NULL)
            if (strcmp(lastname, student->lastname) != 0)
                continue;
        if (name != NULL)
            if (strcmp(name, student->name) != 0)
                continue;
        if (id != NULL)
            if (*id != student->id)
                continue;
        if (course != NULL)
            if (*course != student->course)
                continue;
        if (group != NULL)
            if (strcmp(group, student->group) != 0)
                continue;
        if (marks != NULL)
            if (!isMarksEqual(marks, student->marks))
                continue;

        res[count] = student;
        count++;
    }
    if (resCapacity != count) {
        Student **newRes = (Student **) realloc(res, sizeof(Student *) * count);
        if (newRes == NULL) {
            free(res);
            return 3;
        }
        res = newRes;
    }

    *result = res;
    *resLen = count;
    return 0;
}

int groupByCourse(Student **list, int len, Student ***result, int *resLen, int course) {
    Student **res;
    int rlen;
    int statusCode = findStudents(list, len, &res, &rlen, NULL, NULL, NULL, &course, NULL, NULL);
    if (statusCode != 0)
        return statusCode;
    *result = res;
    *resLen = rlen;
    return 0;
}

short saveToFiles(char *origFilename, Student **list, int len) {
    if (*list == NULL)
        return 1;
    FILE *file1, *file2, *file3, *file4;
    int lenFilename = strlen(origFilename) + 3;
    char *filename = (char *) malloc(sizeof(char) * lenFilename);
    if (filename == NULL)
        return 2;
    strcpy(filename, origFilename);
    filename[lenFilename - 1] = '\0';
    filename[lenFilename - 2] = '1';
    filename[lenFilename - 3] = '_';
    if ((file1 = fopen(filename, "w")) == NULL) {
        free(filename);
        return 3;
    }
    filename[lenFilename - 2] = '2';
    if ((file2 = fopen(filename, "w")) == NULL) {
        fclose(file1);
        free(filename);
        return 4;
    }
    filename[lenFilename - 2] = '3';
    if ((file3 = fopen(filename, "w")) == NULL) {
        fclose(file1);
        fclose(file2);
        free(filename);
        return 5;
    }
    filename[lenFilename - 2] = '4';
    if ((file4 = fopen(filename, "w")) == NULL) {
        fclose(file1);
        fclose(file2);
        fclose(file3);
        free(filename);
        return 6;
    }
    free(filename);

    int avMark = studentsAverageMark(list, len);
    FILE *file;

    for (int x = 0; x < len; x++) {
        Student *student = list[x];
        int studentAvMark = averageMark(student->marks);
        if (studentAvMark <= avMark)
            continue;
        switch (student->course) {
            case 1:
                file = file1;
                break;
            case 2:
                file = file2;
                break;
            case 3:
                file = file3;
                break;
            case 4:
                file = file4;
                break;
        }
        if (fprintf(file, "%s\t%s\t%d\t%s\t%d\n", student->lastname,
                    student->name, student->course, student->group,
                    studentAvMark) < 0) {
            fclose(file1);
            fclose(file2);
            fclose(file3);
            fclose(file4);
            return 7;
        }
    }
    fclose(file1);
    fclose(file2);
    fclose(file3);
    fclose(file4);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        return 69;

    FILE *data;
    Student **list;
    int len;
    char *filename = argv[1];

    if ((data = fopen(filename, "r")) == NULL) {
        return 1;
    }

    int statusCode = readStudents(data, &list, &len);
    if (statusCode != 0) {
        fclose(data);
        return 69;
    }
    sortStudents(list, len, compareByName);
    printStudents(list, len);

    Student **groupList;
    int groupLen;

    statusCode = groupByCourse(list, len, &groupList, &groupLen, 1);
    if (statusCode != 0) {
        killStudents(list, len);
        fclose(data);
        return 69;
    }
    printStudents(groupList, groupLen);

    statusCode = saveToFiles(filename, list, len);
    if (statusCode != 0) {
        free(groupList);
        killStudents(list, len);
        fclose(data);
        return 69;
    }

    free(groupList);
    killStudents(list, len);
    fclose(data);
    return 0;
}