#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "helpers.h"

int main(void)
{
    unsigned char value = 0x81;

    /* Test bits that should be set */
    assert(check_bit(value, 0u) == 1);
    assert(check_bit(value, 7u) == 1);

    /* Test a bit that should be clear */
    assert(check_bit(value, 5u) == 0);

    /* Setting bit 5 should produce 0xA1 */
    assert(set_bit(value, 5u) == 0xA1);

    /* Setting an already-set bit should not change the value */
    assert(set_bit(value, 0u) == value);

    /* Invalid positions should be handled safely. */
    assert(check_bit(value, CHAR_BIT) == 0u);
    assert(set_bit(value, CHAR_BIT) == value);

    /* Clear bit 5.*/
    assert(clear_bit(0xA1u, 5u) == 0x81u);

    /* Clearing an already-clear bit should not change the value */
    assert(clear_bit(value, 5u) == value);

    /* Invalid positions should be handled safely. */
    assert(clear_bit(0x81u, CHAR_BIT) == value);

    /* Toggle clear bit to 1 */
    assert(toggle_bit(0x81u, 5u) == 0xA1u);

    /* Toggle a set bit to 0 */
    assert(toggle_bit(0xA1u, 5u) == 0x81u);

    /* Toggling twice should return the original value */
    assert(toggle_bit(toggle_bit(0x81u, 5u), 5u) == 0x81u);

    /* Invalid positions leave the value unchanged */
    assert(toggle_bit(0x81u, CHAR_BIT) == 0x81u);

    puts("All helper tests passed");

    return 0;
}