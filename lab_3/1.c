#include <stdio.h>


int subtract(int x, int y) {
    while (y != 0) {
        int borrow = (~x) & y;
        x = x ^ y;
        y = borrow << 1;
    }
    return x;
}

int add(int a, int b) {
    int carry;
    while (b != 0) {
        carry = a & b;
        a = a ^ b;
        b = carry << 1;
    }
    return a;
}

int pow2(int r) {
    return 1 << r;
}

int mod(int num, int r) {
    return num & subtract(pow2(r), 1);
}

int division(int dividend, int divisor, int origdiv, int *remainder) {
    int quotient = 1;
    if (dividend == divisor) {
        *remainder = 0;
        return 1;
    } else if (dividend < divisor) {
        *remainder = dividend;
        return 0;
    }
    while (divisor <= dividend) {
        divisor = divisor << 1;
        quotient = quotient << 1;
    }
    if (dividend < divisor) {
        divisor >>= 1;
        quotient >>= 1;
    }
    quotient = add(quotient,
                   division(subtract(dividend, divisor), origdiv, origdiv, remainder));
    return quotient;
}

short toBinary(char *result, int resLen, int num, int r) {
    if (resLen < 2) {
        return 1;
    }
    if (num < 0) {
        return 2;
    }
    if (r < 1) {
        return 1;
    }
    for (int x = 0; x < resLen; x = add(x, 1)) {
        result[x] = '0';
    }
    result[subtract(resLen, 1)] = '\0';
    int base = pow2(r);
    int rem;
    resLen = subtract(resLen, 2);
    while (num >= base) {
        num = division(num, base, base, &rem);
        result[resLen] = (char) add(rem, '0');
        resLen = subtract(resLen, 1);
        if (resLen < 0) {
            return 1;
        }
    }
    if (num > 0) {
        result[resLen] = (char) add(num, '0');
    }
    return 0;
}

int main() {
    char res[10];
    int code = toBinary(res, 10, 3333, 3);
    switch (code) {
        case 1: {
            printf("Too small array\n");
            return 1;
        }
        case 2: {
            printf("Num must be positive\n");
            return 1;
        }
        case 3: {
            printf("'r' must be > 0\n");
            return 1;
        }
    }
    printf("%s\n", res);
    return 0;
}
