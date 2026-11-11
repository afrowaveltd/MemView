#ifndef MEMVIEW_HELPERS_H
#define MEMVIEW_HELPERS_H

/*
 * Check whether a specific bit is set.
 * Returns 1 if set, otherwise 0.
 */

int check_bit(unsigned char value, unsigned int position);

/* 
 * Set a specific bit to 1
 * Returns the modified byte.
 */

unsigned char set_bit(unsigned char value, unsigned int position);

#endif
