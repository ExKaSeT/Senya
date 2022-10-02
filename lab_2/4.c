#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>


double customPow(double number, int power) {
    if (power == 0) {
        return 1;
    } else if (power > 0) {
        return number * customPow(number, power - 1);
    } else if (power < 0) {
        return (1 / number) * customPow(number, power + 1);
    }
}

double avgGeom(int count, ...) {
    double result = 1.0;
    va_list args;
    va_start(args, count);
    for (int x = 0; x < count; x++) {
        result *= va_arg(args, double);
    }
    return pow(result, 1.0 / count);
}

int main(int argc, char *argv[]) {
    printf("%f\n", avgGeom(3, 2.0, 4.0, 8.0));
    printf("%f\n%f\n", customPow(5, 3), customPow(5, -2));
    return 0;
}