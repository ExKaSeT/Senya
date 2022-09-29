#include <stdio.h>
#include <string.h>
#include <math.h>

double strToDouble(char* number) {
    int power = 0;
    double res = 0;
    int sign;
    if (number[0] == '-') {
        sign = -1;
    } else {
        sign = 1;
    }
    for (int i = 0; number[i] != '\0'; i++, power++) {
        if (number[i] == ',' || number[i] == '.') {
            power = 0;
            continue;
        }
        if (i == 0 && number[i] == '-') {
            continue;
        }
        res += number[i] - '0';
        res *= 10;
    }
    res /= 10;

    if (power < strlen(number)) {
        for (int i = 1; i < power; i++) {
            res /= 10;
        }
    }
    res *= sign;
    return res;
}

int strToInt(char* number) {
    int res = 0, sign = 1;
    for (int i = 0; number[i] != '\0'; i++) {
        if (number[i] == '-' && i == 0) {
            sign = -1;
            continue;
        }
        res += number[i] - '0';
        res *= 10;
    }
    res /= 10;
    res *= sign;
    return res;
}

int symbolInDouble(char* number) {
    for (int i = 0; number[i] != '\0'; i++) {
        if (number[i] == '-' && i == 0) {
            continue;
        }
        if ((number[i] == ',' || number[i] == '.') && i != 0) {
            continue;
        }
        if (number[i] > '9' || number[i] < '0') {
            return 1;
        }
    }
    return 0;
}

int symbolInInt(char* number) {
    for (int i = 0; number[i] != '\0'; i++) {
        if (number[i] == '-' && i == 0) {
            continue;
        }
        if (number[i] > '9' || number[i] < '0') {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("No arguments\n");
        return 0;
    }
    if (strcmp(argv[1], "-q") == 0 || strcmp(argv[1], "/q") == 0) {
        if (argc != 5) {
            printf("You must enter 3 parameters with flag '-q'\n");
            return 0;
        } else {
            double a = 0, b = 0, c = 0;
            if (symbolInDouble(argv[2]) || symbolInDouble(argv[3]) ||
                symbolInDouble(argv[4])) {
                printf("You must enter only numbers with flag '-q'\n");
                return 0;
            }
            a = strToDouble(argv[2]);
            b = strToDouble(argv[3]);
            c = strToDouble(argv[4]);
            //printf("%f %f %f", a, b, c);
            double eps = 0.00001;
            if (fabs(a) < eps && fabs(b) > eps && fabs(c) > eps) {
                c *= -1;
                double x = 0;
                x = c / b;
                printf("One solution:\n%f\n", x);
                return 0;
            } else if (fabs(a) > eps && fabs(b) < eps && fabs(c) > eps) {
                c *= -1;
                double x_v2 = c / b;
                if (x_v2 < eps) {
                    printf("No solutions\n");
                    return 0;
                }
                double x1 = sqrt(x_v2);
                double x2 = x1 * -1;
                printf("Two solutions:\n%f\n%f\n", x1, x2);
                return 0;
            } else if (fabs(a) < eps && fabs(b) < eps && fabs(c) > eps) {
                printf("No solutions\n");
                return 0;
            } else if ((fabs(a) > eps && fabs(b) < eps && fabs(c) < eps) ||
                       (fabs(a) < eps && fabs(b) > eps && fabs(c) < eps)) {
                double x = 0;
                printf("One solution:\n%f\n", x);
                return 0;
            }
            double discriminant = 0;
            discriminant = b * b - 4 * a * c;
            if (discriminant > eps) {
                double x1 = 0, x2 = 0;
                x1 = (-b + sqrt(discriminant)) / (2 * a);
                x2 = (-b - sqrt(discriminant)) / (2 * a);
                printf("Two solutions:\n%f\n%f\n", x1, x2);
                return 0;
            } else if (fabs(discriminant) < eps) {
                double x = 0;
                x = -(b / (2 * a));
                printf("One solution:\n%f\n", x);
                return 0;
            } else {
                printf("No solutions\n");
                return 0;
            }
        }
    } else if (strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "/m") == 0) {
        if (argc != 4) {
            printf("You must enter 2 parameters with flag '-m'\n");
            return 0;
        } else {
            if (symbolInInt(argv[2]) || symbolInInt(argv[3])) {
                printf("There is a symbol in your number\n");
                return 0;
            }
            int num1 = 0, num2 = 0;
            num1 = strToInt(argv[2]);
            num2 = strToInt(argv[3]);
            if (num1 == 0 || num2 == 0) {
                printf("Enter numbers that are not equal to zero\n");
                return 0;
            }
            if (num1 % num2 == 0) {
                printf("%d is a multiple of %d\n", num1, num2);
            } else {
                printf("%d is not a multiple of %d\n", num1, num2);
            }
            return 0;
        }
    } else if (strcmp(argv[1], "-t") == 0 || strcmp(argv[1], "/t") == 0) {
        if (argc != 5) {
            printf("You must enter 3 parameters with flag '-t'\n");
            return 0;
        } else {
            double a = 0, b = 0, c = 0;
            double eps = 0.0001;
            if (symbolInDouble(argv[2]) || symbolInDouble(argv[3]) ||
                symbolInDouble(argv[4])) {
                printf("You must enter only numbers with flag '-q'\n");
                return 0;
            }
            a = strToDouble(argv[2]);
            b = strToDouble(argv[3]);
            c = strToDouble(argv[4]);
            if (a < eps || b < eps || c < eps) {
                printf("Enter only positive numbers\n");
                return 0;
            }
            if (fabs(a) < eps || fabs(b) < eps || fabs(c) < eps) {
                printf("Enter numbers that are not equal to zero\n");
                return 0;
            }
            if (fabs((a * a + b * b) - (c * c)) < eps ||
                fabs((a * a + c * c) - (b * b)) < eps ||
                fabs((b * b + c * c) - (a * a)) < eps) {
                printf("Yes\n");
            } else {
                printf("No\n");
            }
            return 0;
        }
    } else {
        printf("Wrong argument\n");
        return 0;
    }
}