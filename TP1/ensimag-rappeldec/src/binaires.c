#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static unsigned long long X = 123456ULL;

unsigned char crand48(void) {
    unsigned long long a = 0x5DEECE66D;
    unsigned long long c = 0xB;
    unsigned long long m = pow(2,48);
    X = (a*X+c)%m;
    return 0xFF&(X>>32);
}

int main(void)
{
    printf("%hhu\n", crand48());
    printf("%hhu\n", crand48());
    printf("%hhu\n", crand48());

    return EXIT_SUCCESS;
}
