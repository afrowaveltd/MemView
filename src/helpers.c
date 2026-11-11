#include <limits.h>
#include "helpers.h"

/*
 * Check whether a specific bit is set.
 */

int check_bit(unsigned char value, unsigned int position)
{
	if (position >= CHAR_BIT)
	{
		return 0;
	}

	return ((unsigned int)value & (1u << position)) != 0u;
}

/*
 * Set a specific bit to 1.
 * Returns the modified byte.
 */

unsigned char set_bit(unsigned char value, unsigned int position)
{
	if (position >= CHAR_BIT)
	{
		return value;
	}

	return (unsigned char)(value | (1u << position));
}

/*
 * Clear a specific bit to 0.
 * Returns the modified byte.
 */

unsigned char clear_bit(unsigned char value, unsigned int position)
{
	if (position >= CHAR_BIT)
	{
		return value;
	}

	return (unsigned char)(value & ~(1u << position));
}

/*
 * Toggle a specific bit.
 * Returns the modified byte.
 */

unsigned char toggle_bit(unsigned char value, unsigned int position)
{
	if (position >= CHAR_BIT)
	{
		return value;
	}

	return (unsigned char)(value ^ (1u << position));
}
