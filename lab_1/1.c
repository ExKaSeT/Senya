#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>


long long int factorial(long long int number) {
    long long int result = 1;
    if (number == 0 || number == 1) {
        return 1;
    } else {
        for (int i = 1; i <= number; i++) {
            if (LLONG_MAX / result > i) {
                result *= i;
            } else {
                return -1;
            }
        }
        return result;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Enter two arguments\n");
        return 0;
    } else {
        for (int i = 0; argv[2][i] != '\0'; i++) {
            if (
                    (argv[2][i] >= 48 && argv[2][i] <= 57) &&
                    ((argv[1][0] == 45 || argv[1][0] == 47) &&
                     (argv[1][1] == 97 || argv[1][1] == 101 ||
                      argv[1][1] == 102 || argv[1][1] == 104 ||
                      argv[1][1] == 112 || argv[1][1] == 115))
            ) {
                printf("The number must be the first, then the flag\n");
                return 0;
            }
        }

        int num = 0;
        if (argv[1][0] == 45) {
            printf("Your number must be positive\n");
            return 0;
        } else {
            for (int i = 0; argv[1][i] != '\0'; i++) {
                if(argv[1][i] < 48 || argv[1][i] > 57){
                    printf("Delete a symbol in your number\n");
                    return 0;
                }
            }
        }
        // string -> int
        for (int i = 0; argv[1][i] != '\0'; i++) {
            num += argv[1][i] - 48;
            num *= 10;
        }
        num /= 10;

        if (argv[2][0] != 45 && argv[2][0] != 47) {
            printf("There must be symbol '-' or '/' before the flag\n");
            return 0;
        }

        if (strcmp(argv[2], "-h") == 0 || strcmp(argv[2], "/h") == 0) {
            if (num > 100) {
                printf("No multiples\n");
            } else if (num == 0) {
                printf("Divide by zero\n");
            } else {
                for (int i = num; i <= 100; i++) {
                    if (i % num == 0) {
                        printf("%d ", i);
                    }
                }
            }
            return 0;
        } else if (strcmp(argv[2], "-p") == 0 || strcmp(argv[2], "/p") == 0) {
            if (num == 1 || num == 0) {
                printf("Enter the number other than one or zero\n");
                return 0;
            } else {
                for (int i = 2; i < floor(sqrt(num)); i++) {
                    if (num % i == 0) {
                        printf("Your number is composite\n");
                        return 0;
                    }
                }
                printf("Your number is prime\n");
                return 0;
            }
        } else if (strcmp(argv[2], "-s") == 0 || strcmp(argv[2], "/s") == 0) {
            for (int i = 0; argv[1][i] != '\0'; i++) {
                printf("%c ", argv[1][i]);
            }
        } else if (strcmp(argv[2], "-e") == 0 || strcmp(argv[2], "/e") == 0) {
            if (num == 0) {
                printf("Enter number more than 0\n");
                return 0;
            } else if (num > 10) {
                printf("Enter number less than 11\n");
                return 0;
            } else {
                long long int res = 0;
                for (int i = 1; i < 11; i++) {
                    res = i;
                    printf("%d: ", i);
                    for (int j = 1; j <= num; j++) {
                        for (int k = 1; k < j; k++) {
                            res *= i;
                        }
                        printf("%lld ", res);
                        res = i;
                    }
                    printf("\n");
                }
                return 0;
            }
        } else if (strcmp(argv[2], "-a") == 0 || strcmp(argv[2], "/a") == 0) {
            if (num == 0) {
                printf("Enter number more than 0\n");
                return 0;
            } else {
                int res = 0;
                res = (num * (num + 1)) / 2;
                printf("%d\n", res);
                return 0;
            }
        } else if (strcmp(argv[2], "-f") == 0 || strcmp(argv[2], "/f") == 0) {
            long long int numLong = num;
            numLong = factorial(numLong);
            if (numLong == -1) {
                printf("Overflow\n");
                return 0;
            }
            printf("%lld\n", numLong);
            return 0;
        } else {
            printf("Wrong argument\n");
            return 0;
        }
    }
}