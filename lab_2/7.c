#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>


short isConvex(int len, ...) {
    if (len < 3) {
        return 0;
    }
    va_list args;
    int coords[2][len];
    va_start(args, len);
    for (int i = 0; i < len; i++) {
        coords[0][i] = va_arg(args, int);
        coords[1][i] = va_arg(args, int);
        printf("X: %d Y: %d\n", coords[0][i], coords[1][i]);
    }
    int Q = 1;
    int T = coords[0][len - 1] * coords[1][0] - coords[0][0] * coords[1][len - 1];
    int Z = T / fabs(T);
    int P = 1;
    int i = 1;
    while (i < len - 1 && Q) {
        T = coords[0][i] * coords[1][i + 1] - coords[0][i + 1] * coords[1][i];
        P = P * Z * T / fabs(T);
        if (P < 0)
            Q = 0;
        i++;
    }
    va_end(args);
    if (Q)
        return 1;
    else
        return 0;
}

double solve(int x, int n, ...) {
    va_list args;
    double result = 0;
    va_start(args, n);
    int arg;
    for (int i = n; i >= 0; i--) {
        arg = va_arg(args, int);
        result += pow(x, i) * arg;
    }
    va_end(args);
    return result;
}

int main() {
    if (isConvex(6, 0, 0, 0, 1, 1, 2, 2, 0, 1, -2, -2, -1))
        printf("convex\n");
    else
        printf("not convex\n");
    if (isConvex(6, 0, 0, 0, 1, 2, 2, 3, 0, 2, -2, 1, -2))
        printf("convex\n");
    else
        printf("not convex\n");
    printf("%f\n", solve(2, 2, 2, 2, 2));
    return 0;
}