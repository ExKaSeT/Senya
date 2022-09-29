#include <stdio.h>
#include <string.h>
#include <ctype.h>


void argI(FILE* f1, FILE* f2) {
    long long int counter = 0;
    char c;
    while ((c = getc(f1)) != EOF) {
        if (c == '\n') {
            fprintf(f2, "%lld\n", counter);
            counter = 0;
        }
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            counter += 1;
        }
    }
    fprintf(f2, "%lld", counter);
}

void argD(FILE* f1, FILE* f2) {
    char c;
    while ((c = getc(f1)) != EOF) {
        if (c < 48 || c > 57) {
            putc(c, f2);
        }
    }
}

void argS(FILE* f1, FILE* f2) {
    long long int counter = 0;
    char c;
    while ((c = getc(f1)) != EOF) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              c == ' ' || c == '\n' || c == ' ' || c == '\t')) {
            counter += 1;
        }
        if (c == '\n') {
            fprintf(f2, "%lld\n", counter);
            counter = 0;
        }
    }
    fprintf(f2, "%lld", counter);
}

void argA(FILE* f1, FILE* f2) {
    char c;
    int num = 0;
    while ((c = getc(f1)) != EOF) {
        if (c == '\n' || c == ' ' || c == '\t') {
            putc(c, f2);
            continue;
        }
        if (c < '0' || c > '9') {
            num = c;
            fprintf(f2, "%d", num);
        } else {
            putc(c, f2);
        }
    }
}

void argF(FILE* f1, FILE* f2) {
    char c;
    char _c;
    int num = 0;
    int counter = 0;
    while ((c = getc(f1)) != EOF) {
        if ((c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z') &&
            !(_c >= 'A' && _c <= 'Z' || _c >= 'a' && _c <= 'z')) {
            counter += 1;
            if (counter % 2 == 0 && counter % 5 == 0) {
                while (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z') {
                    putc(tolower(c), f1);
                    num = c;
                    fprintf(f2, "%d", num);
                    _c = c;
                    c = getc(f1);
                }
                putc(c, f2);
                continue;
            }
            if (counter % 2 == 0) {
                while (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z') {
                    putc(tolower(c), f2);
                    _c = c;
                    c = getc(f1);
                }
                putc(c, f2);
                continue;
            } else if (counter % 5 == 0) {
                while (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z') {
                    num = c;
                    fprintf(f2, "%d", num);
                    _c = c;
                    c = getc(f1);
                }
                putc(c, f2);
                continue;
            }
        }
        putc(c, f2);
        _c = c;
    }
}

void copyFile(FILE* f1, FILE* f2) {
    char c;
    while ((c = getc(f2)) != EOF) {
        putc(c, f1);
    }
}

int main(int argc, char* argv[]) {
    FILE *fin, *fout;
    if (argc == 1) {
        printf("No arguments\n");
        return 0;
    }
    fin = fopen(argv[2], "r");
    if (!fin) {
        printf("Can't open the file\n");
        return 0;
    }
    char prefix[] = "out_";
    char* filename = strcat(prefix, argv[2]);

    if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "/d") == 0) {
        if (argc != 3) {
            printf("Enter only path with flag '-d'\n");
            return 0;
        }
        fout = fopen(filename, "w");
        if (!fout) {
            printf("Can't open the file\n");
            return 0;
        }
        argD(fin, fout);
        fclose(fin);
        fclose(fout);
        printf("Done\n");
        return 0;

    } else if (strcmp(argv[1], "-nd") == 0 || strcmp(argv[1], "/nd") == 0) {
        if (argc == 4) {
            fout = fopen(argv[3], "w");
            if (!fout) {
                printf("Can't open the file\n");
                return 0;
            }
            argD(fin, fout);
        } else {
            printf("You must enter arguments like this:\n-n* input output\n");
            return 0;
        }
        fclose(fin);
        fclose(fout);
        printf("Done\n");
        return 0;

    } else if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "/i") == 0) {
        if (argc != 3) {
            printf("Enter only path with flag '-i'\n");
            return 0;
        }
        fout = fopen(filename, "w");
        if (!fout) {
            printf("Can't open the file\n");
            return 0;
        }
        argI(fin, fout);
        fclose(fin);
        fclose(fout);
        printf("Done\n");
        return 0;

    } else if (strcmp(argv[1], "-ni") == 0 || strcmp(argv[1], "/ni") == 0) {
        if (argc == 4) {
            fout = fopen(argv[3], "w");
            if (!fout) {
                printf("Can't open the file\n");
                return 0;
            }
            argI(fin, fout);
        } else {
            printf("You must enter arguments like this:\n-n* input output\n");
            return 0;
        }
        fclose(fin);
        fclose(fout);
        printf("Done\n");
        return 0;

    } else if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "/s") == 0) {
        if (argc != 3) {
            printf("Enter only path with flag '-s'\n");
            return 0;
        }
        fout = fopen(filename, "w");
        if (!fout) {
            printf("Can't open the file\n");
            return 0;
        }
        argS(fin, fout);
        fclose(fin);
        fclose(fout);
        printf("Done\n");
        return 0;

    } else if (strcmp(argv[1], "-ns") == 0 || strcmp(argv[1], "/ns") == 0) {
        if (argc == 4) {
            fout = fopen(argv[3], "w");
            if (!fout) {
                printf("Can't open the file\n");
                return 0;
            }
            argS(fin, fout);
        } else {
            printf("You must enter arguments like this:\n-n* input output\n");
            return 0;
        }
        fclose(fin);
        fclose(fout);
        printf("Done\n");
        return 0;

    } else if (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "/a") == 0) {
        if (argc != 3) {
            printf("Enter only path with flag '-a'\n");
            return 0;
        }
        fout = fopen(filename, "w");
        if (!fout) {
            printf("Can't open the file\n");
            return 0;
        }
        argA(fin, fout);
        fclose(fin);
        fclose(fout);
        printf("Done\n");
        return 0;

    } else if (strcmp(argv[1], "-na") == 0 || strcmp(argv[1], "/na") == 0) {
        if (argc == 4) {
            fout = fopen(argv[3], "w");
            if (!fout) {
                printf("Can't open the file\n");
                return 0;
            }
            argA(fin, fout);
        } else {
            printf("You must enter arguments like this:\n-n* input output\n");
            return 0;
        }
        fclose(fin);
        fclose(fout);
        printf("Done\n");
        return 0;

    } else if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "/f") == 0) {
        if (argc != 3) {
            printf("Enter only path with flag '-f'\n");
            return 0;
        }
        fout = fopen(filename, "w");
        if (!fout) {
            printf("Can't open the file\n");
            return 0;
        }
        argF(fin, fout);
        fclose(fin);
        fclose(fout);
        printf("Done\n");
        return 0;

    } else if (strcmp(argv[1], "-nf") == 0 || strcmp(argv[1], "/nf") == 0) {
        if (argc == 4) {
            fout = fopen(argv[3], "w");
            if (!fout) {
                printf("Can't open the file\n");
                return 0;
            }
            argF(fin, fout);
        } else {
            printf("You must enter arguments like this:\n-n* input output\n");
            return 0;
        }
        fclose(fin);
        fclose(fout);
        printf("Done\n");
        return 0;

    } else {
        printf("Wrong flag\n");
        return 0;
    }
}