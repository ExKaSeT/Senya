#include <stdio.h>
#include <string.h>


int isNumber(char c) {
    if (c >= '0' && c <= '9')
        return 1;
    return 0;
}

int isLetter(char c) {
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
        return 1;
    return 0;
}

int main() {
    int base;
    int max = 0;
    char numbers[34];
    char maxNumber[34];
    printf("Enter the number system: ");
    scanf("%d", &base);
    if (base < 2 || base > 36) {
        printf("Incorrect number system\n");
        return 1;
    }
    printf("Enter numbers:\n");
    while (strcmp(numbers, "Stop")) {
        scanf("%s", numbers);
        char* pb = numbers - 1;
        int result = 0;
        while (*++pb) {
            if ((isNumber(*pb) && (*pb - '0' >= base) ||
                 isLetter(*pb) && *pb - 'A' + 10 >= base) &&
                strcmp(numbers, "Stop")) {
                printf("Incorrect input: %c", *pb);
                return 1;
            }
            if (isNumber(*pb)) {
                result = result * base + *pb - '0';
            } else {
                result = result * base + *pb - 'A' + 10;
            }
        }
        if (result > max && strcmp(numbers, "Stop")) {
            max = result;
            strcpy(maxNumber, numbers);
        }
    }
    printf("Max number in %d system: %s\n", base, maxNumber);
    printf("In decimal number system: %d\n", max);
    char str[34];
    for (int i = 9; i <= 36; i += 9) {
        int r;
        base = i;
        int temp = max;
        int j = 0;
        while (temp) {
            r = temp % base;
            if (r > 9)
                str[j] = r - 10 + 'A';
            else
                str[j] = r + '0';
            temp /= base;
            j++;
        }
        printf("In %d number system: ", base);
        for (int n = j - 1; n >= 0; n--) {
            printf("%c", str[n]);
        }
        printf("\n");
    }
    return 0;
}