#include <stdlib.h>
#include <stdio.h>
#include <math.h>


long double limit(long double lim(int n), double eps) {
    int i = 1 / eps / eps, step = 1 / eps;
    long double res = 0, prevRes = 0;
    eps *= eps;
    while (fabs(res - prevRes) > eps || prevRes == 0) {
        prevRes = res;
        res = lim(i += step);
    }
    return res;
}

long double sum(long double function(int n), double eps, int start) {
    long double res = 0, epsFunc = eps * eps, prevRes = 0;
    for (start; (fabs(res - prevRes) > eps || fabs(prevRes) < eps || (res - prevRes == 0)); start++) {
        prevRes = res;
        res += function(start);
    }
    return res;
}

long double multipl(long double function(int n), double eps, int start) {
    long double res = 1, prevRes = 0, epsFunc = eps / 10;
    for (start; fabs(prevRes - res) > epsFunc || fabs(prevRes) < eps; start++) {
        prevRes = res;
        res *= function(start);
    }
    return res;
}

long double factorial(int num) {
    if (num == 0) {
        return 1;
    }
    int res = 1;
    while (num > 0) {
        res *= num;
        num--;
    }
    return res;
}

long double eLim(int n) {
    return pow(1 + 1.0 / n, n);
}

long double piLim(int n) {
    int tmp = n;
    long double res = 1;
    while (tmp) {
        res *= (double) tmp / (tmp - 0.5);
        tmp--;
    }
    res /= sqrt(n);
    return res *= res;
}

long double ln2Lim(int n) {
    return n * (pow(2, 1.0 / n) - 1);
}

long double sqrt2Lim(double eps) {
    int i = 0, j;
    long double res = -0.5, prevRes = 0;
    eps *= eps;

    while (fabs(res - prevRes) > eps || prevRes == 0) {
        prevRes = res;
        res = res - ((res * res) / 2) + 1;
    }

    return res;
}

long double eulerLim(int n) {
    long double sum = 0;
    for (int k = 1; k <= n; k++) {
        sum += 1.0 / k;
    }
    return -log(n) + sum;
}

long double eRow(int n) {
    return 1.0 / factorial(n);
}

long double piRow(int n) {
    return 4 * pow(-1.0, n - 1) / (2.0 * n - 1);
}

long double ln2Row(int n) {
    return pow(-1.0, n - 1) / n;
}

long double sqrt2Multipl(int k) {
    return pow(2, pow(2, -k));
}

long double gammaRow(double eps, int start) {
    long double res = 0, epsFunc = 1e-15, prevRes = 0, funcReturned = 0;
    for (start; (fabs(res - prevRes) > epsFunc || fabs(prevRes) < eps || (res - prevRes == 0)); start++) {
        prevRes = res;
        funcReturned = 1.0 / pow(floor(sqrt(start)), 2) - 1.0 / start;
        res += funcReturned;
    }
    return res;
}

double equationDichotomy(double equation(double x), double a, double b, double eps) {
    double functResultMiddle;
    while (fabs(a - b) > eps) {
        functResultMiddle = equation((a + b) / 2);
        if (equation(a) * functResultMiddle > 0) {
            a = (a + b) / 2;
        }

        if (equation(b) * functResultMiddle > 0) {
            b = (a + b) / 2;
        }
    }
    return (a + b) / 2;
}

double eEquation(double x) {
    return log(x) - 1;
}

double piEquation(double x) {
    return -sin(x);
}

double ln2Equation(double x) {
    return pow(M_E, x) - 2;
}

double sqrt2Equation(double x) {
    return x * x - 2;
}

void simpleNumsGenerate(long **simpleNums, unsigned long long toGenerate, unsigned long long *generated) {
    int count = 0;
    long *tmp = NULL, i, j, *arraySieve = NULL;

    if (!(arraySieve = (long *) calloc((toGenerate + 1), sizeof(long)))) {
        return;
    }
    for (i = 2; i <= toGenerate; i++) {
        if (!arraySieve[i]) {
            count++;
            for (j = i + i; j <= toGenerate; j += i) {
                arraySieve[j] = 1;
            }
        }
    }
    if ((*simpleNums == NULL || (*generated) == 0)) {
        if (!((*simpleNums) = (long *) calloc(count, sizeof(long)))) {
            free(arraySieve);
            return;
        }
    } else {
        if (!(tmp = (long *) realloc((*simpleNums), sizeof(long) * (count + (*generated))))) {
            free(arraySieve);
            free(*simpleNums);
            return;
        }
        (*simpleNums) = tmp;
    }
    i = (!(*generated) ? 2 : (*simpleNums)[(*generated) - 1] + 1);
    for (i; i <= toGenerate; i++) {
        if (!arraySieve[i]) {
            (*simpleNums)[(*generated)++] = i;
        }
    }
    free(arraySieve);
}

double eulerEquation(double eps) {
    double result = 1, prevResult = 0;
    int step = 1.0 / eps / 100;
    long *simpleNums = NULL;
    unsigned long long t = 1.0 / eps / eps, generated = 0, i;
    for (t; fabs(result - prevResult) > eps; t += step) {
        prevResult = result;
        result = 1;
        simpleNumsGenerate(&simpleNums, t, &generated);
        for (i = 0; i < generated; i++) {
            result *= (double) (simpleNums[i] - 1) / simpleNums[i];
        }
        result *= log(t);
        result = -log(result);
    }
    free(simpleNums);
    return result;
}

int main() {
    int l = 3;
    double eps = pow(10, -l);

    printf("e\n");
    printf("limit: %.*Lf\n", l, limit(eLim, eps));
    printf("row sum: %.*Lf\n", l, sum(eRow, eps, 0));
    printf("equation: %.*lf\n\n", l, equationDichotomy(eEquation, 2., 3., eps));

    printf("pi:\n");
    printf("limit: %.*Lf\n", l, limit(piLim, eps));
    printf("row sum: %.*Lf\n", l, sum(piRow, eps, 1));
    printf("equation: %.*lf\n\n", l, equationDichotomy(piEquation, 3, 3.5, eps));

    printf("ln(2):\n");
    printf("limit: %.*Lf\n", l, limit(ln2Lim, eps));
    printf("row sum: %.*Lf\n", l, sum(ln2Row, eps, 1));
    printf("equation: %.*lf\n\n", l, equationDichotomy(ln2Equation, 0, 1, eps));

    printf("sqrt(2):\n");
    printf("limit: %.*Lf\n", l, sqrt2Lim(eps));
    printf("row product: %.*Lf\n", l, multipl(sqrt2Multipl, eps, 2));
    printf("equation: %.*lf\n\n", l, equationDichotomy(sqrt2Equation, 1, 2, eps));

    printf("Euler-Mascheroni:\n");
    printf("limit: %.*Lf\n", l, limit(eulerLim, eps));
    printf("row sum: %.*Lf\n", l, gammaRow(eps, 2) - M_PI * M_PI / 6);
    printf("equation: %.*lf\n\n", l, eulerEquation(eps));

    return 0;
}