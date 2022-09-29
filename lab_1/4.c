#include <stdio.h>

int main(int argc, char* argv[]) {
    FILE *fin, *fout;
    char str1[100], str2[100], str3[100];
    if (argc != 2) {
        printf("Enter filename\n");
        return 0;
    }

    fin = fopen(argv[1], "r");
    if (!fin) {
        printf("Can't open the file\n");
        return 0;
    }
    fout = fopen("temp.txt", "w");

    while (fscanf(fin, "%s%s%s", str1, str2, str3) != EOF) {
        fprintf(fout, "%s %s %s\n", str3, str1, str2);
    }

    fclose(fin);
    fclose(fout);
    remove(argv[1]);
    rename("temp.txt", argv[1]);
    return 0;
}