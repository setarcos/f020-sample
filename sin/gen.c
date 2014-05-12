#include <math.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    for (int i = 0; i < 20; ++i) {
        printf("%3.0f, ", sinf(3.14159 /10.0 * i) * 0x4000 + 0x8000);
    }
    printf("\n");
    printf("%f\n", sinf(0));
}
