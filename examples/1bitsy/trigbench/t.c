#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_TERMS 7

const float coeff[MAX_TERMS] = {
    +0x1.000000p+0,
    -0x1.555556p-3,
    +0x1.111112p-7,
    -0x1.a01a02p-13,
    +0x1.71de3ap-19,
    -0x1.ae6456p-26,
    +0x1.612462p-33,
};

void init_coeff(void)
{
    double s = +1.0;
    int64_t f = 1;
    for (size_t i = 0; i < MAX_TERMS; i++) {
        float z = (float)(s / (double)f);
        if (z != coeff[i]) {
            printf("mismatch: %g %a\n", z, z);
        }
        printf("    %+a,\n", coeff[i]);
        f *= 2 * i + 2; f *= 2 * i + 3;
        s = -s;
    }
}

float taylor_sin_xx(float x, unsigned count)
{
    float y = 0;
    float x2 = x * x;
    int s = +1;
    int f = 1;
    float xp = x;
    for (unsigned i = 0; i < count; i++) {
        y +=  s * xp / (float)f;
        // printf("%u: s, f, xp = %d, %d, %g\n", i, s, f, xp);
        f *= 2 * i + 2; f *= 2 * i + 3;
        s = -s;
        xp *= x2;
    }
    return y;
}

float taylor_sin_xx1(float x)
{
    static const float coeff[] = {
        +0x1.000000p+0,
        -0x1.555556p-3,
        +0x1.111112p-7,
        -0x1.a01a02p-13,
        +0x1.71de3ap-19,
        -0x1.ae6456p-26,
        +0x1.612462p-33,
    };

    float y = 0;
    float x2 = x * x;
    float xp = x;
    for (unsigned i = 0; i < 7; i++) {
        y += xp * coeff[i];
        xp *= x2;
    }
    return y;
}

float taylor_sin_xx2(float x)
{
    static const float coeff[] = {
        +0x1.000000p+0,
        -0x1.555556p-3,
        +0x1.111112p-7,
        -0x1.a01a02p-13,
        +0x1.71de3ap-19,
        -0x1.ae6456p-26,
        +0x1.612462p-33,
    };

    float y = 0;
    float x2 = x * x;
    float xp = x;
    y += xp * coeff[0];
    xp *= x2;
    y += xp * coeff[1];
    xp *= x2;
    y += xp * coeff[2];
    xp *= x2;
    y += xp * coeff[3];
    xp *= x2;
    y += xp * coeff[4];
    xp *= x2;
    y += xp * coeff[5];
    xp *= x2;
    y += xp * coeff[6];
    xp *= x2;
    return y;
}

float taylor_sin(float x)
{
    const float coeff1 = -0x1.555556p-3;
    const float coeff2 = +0x1.111112p-7;
    const float coeff3 = -0x1.a01a02p-13;
    const float coeff4 = +0x1.71de3ap-19;
    const float coeff5 = -0x1.ae6456p-26;
    const float coeff6 = +0x1.612462p-33;

    float y = 0;
    float x2 = x * x;
    float xp = x;
    y += xp;
    xp *= x2;
    y += xp * coeff1;
    xp *= x2;
    y += xp * coeff2;
    xp *= x2;
    y += xp * coeff3;
    xp *= x2;
    y += xp * coeff4;
    xp *= x2;
    y += xp * coeff5;
    xp *= x2;
    y += xp * coeff6;
    xp *= x2;
    return y;
}

int main()
{
    init_coeff();

    for (int j = 0; j < 10; j++) {
        printf("j = %d\n", j);
        float x = (float)j * M_PI / 2 / 10;
        printf("sin(%g) = %g\n", x, sin(x));
        printf("sin(%g) = %g\n", x, taylor_sin(x));
    }

    
    return 0;
}
