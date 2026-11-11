
#include <stdio.h>
#include "helpers.h"

int main(void)
{
    unsigned char value = 0x81;

    printf("Before: 0x%02X\n", (unsigned int)value);
    printf("Bit 5: %d\n", check_bit(value, 5u));

    value = set_bit(value, 5u);

    printf("After:  0x%02X\n", (unsigned int)value);
    printf("Bit 5: %d\n", check_bit(value, 5u));

    return 0;
}
