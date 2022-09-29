#include <stdio.h>
#include <string.h>
#include <stdbool.h>


void commandFi(FILE* file, char filenames[][100], int* size) {
    while (fscanf(file, "%s", filenames[*size]) != EOF) {
        (*size)++;
    }
    fclose(file);
}

void commandCin(char filenames[][100], int* size) {
    printf("To stop entering enter 'stop'\n");
    while (scanf("%s", filenames[*size]) != EOF &&
           strcmp(filenames[*size], "stop") != 0) {
        (*size)++;
    }
}

void commandArg(char* argv[], char filenames[][100], int size) {
    int endNumber = 0;
    for (int i = 2; i < size + 2; i++) {
        for (int j = 0; argv[i][j] != '\0'; j++) {
            filenames[i - 2][j] = argv[i][j];
            endNumber = j;
        }
        filenames[i][endNumber] = '\0';
    }
}

bool isFilesOpened(bool* opened, int size) {
    for (int i = 0; i < size; i++) {
        if (opened[i] == true) {
            return true;
        }
    }
    return false;
}

void writing(FILE* resf, char filenames[][100], int size) {
    FILE* files[100];
    bool opened[size];
    resf = fopen("out.txt", "w");
    char c;
    int flag = 0;

    for (int i = 0; i < size; i++) {
        opened[i] = true;
        files[i] = NULL;
    }

    while (isFilesOpened(opened, size)) {
        for (int i = 0; i < size; i++) {
            if (files[i] == NULL) {
                files[i] = fopen(filenames[i], "r");
                // проверяем файлы на открытие
                if (!files[i]) {
                    flag = 1;
                    fclose(resf);
                    remove("out.txt");
                    break;
                }
            }

            c = getc(files[i]);
            if (c == EOF) {
                fclose(files[i]);
                opened[i] = false;
            } else {
                putc(c, resf);
            }

        }
        if (flag) {
            for (int j = 0; j < size; j++) {
                fclose(files[j]);
            }
            printf("Can't open some files\n");
            break;
        }
    }
    if (flag == 0) {
        printf("Done\n");
    }
    fclose(resf);
}

int main(int argc, char* argv[]) {
    FILE *fin, *fout;
    char filenames[100][100];
    int size = 0;
    if (argc == 1) {
        printf("No arguments\n");
        return 0;
    }
    if (strcmp(argv[1], "-fi") == 0) {
        if (argc == 2) {
            printf("Enter the file name\n");
            return 0;
        }
        if (argc > 3) {
            printf("Too many arguments\n");
            return 0;
        }
        fin = fopen(argv[2], "r");
        if (!fin) {
            printf("Can't open file\n");
            return 0;
        }
        commandFi(fin, filenames, &size);
        writing(fout, filenames, size);
        return 0;

    } else if (strcmp(argv[1], "-cin") == 0) {
        if (argc != 2) {
            printf("Don't enter any arguments with this flag\n");
            return 0;
        }
        printf("Enter file names:\n");
        commandCin(filenames, &size);
        if (size == 0) {
            printf("You didn't enter any files\n");
            remove("out.txt");
            return 0;
        }
        writing(fout, filenames, size);
        return 0;

    } else if (strcmp(argv[1], "-arg") == 0) {
        if (argc == 2) {
            printf("Enter file names with this flag\n");
            return 0;
        }
        size = argc - 2;
        commandArg(argv, filenames, size);
        writing(fout, filenames, size);
        return 0;
    } else {
        printf("Wrong argument\n");
        return 0;
    }
}