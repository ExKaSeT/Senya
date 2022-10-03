#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>


double f(double x) {
    return 4 - exp(x) - 2 * pow(x, 2);
}

double dihot(double a, double b, double eps, double (*func)(double)) {
    double c;
    while (b - a > eps) {
        c = (a + b) / 2.0;
        if (func(b) * func(c) < 0)
            a = c;
        else
            b = c;
    }
    return (a + b) / 2.0;
}

int main(int argc, char *argv[]) {
    printf("%f", dihot(0, 2, 0.01, f));
    return 0;
}