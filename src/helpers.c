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

unsigned char set_bit(unsigned char value, unsigned int position)
{
	if (position >= CHAR_BIT)
	{
		return value;
	}

	return (unsigned char)(value | (1u << position));
}
