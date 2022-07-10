#ifndef BEEMU_INTERNALS_UTILITY_H
#define BEEMU_INTERNALS_UTILITY_H

#include <stdbool.h>
#include <stdint.h>

typedef struct BeemuByteTuple
{
	uint8_t first;
	uint8_t second;
} BeemuByteTuple;

/**
 * @brief Check if the given integer is one of the elements.
 *
 * @param element Element to search for.
 * @param n the number of elements
 * @param ... Elements to search in.
 * @return true If the element is one of the elements.
 * @return false If the element is not one of the elements.
 */
bool beemu_util_is_one_of(int element, int n, ...);

/**
 * @brief Check if the given element is one of the two elements.
 *
 * @param element
 * @param x
 * @param y
 * @return true
 * @return false
 */
bool beemu_util_is_one_of_two(int element, int x, int y);

/**
 * @brief Check if the given element is one of the three elements.
 *
 * @param element
 * @param x
 * @param y
 * @param z
 * @return true
 * @return false
 */
bool beemu_util_is_one_of_three(int element, int x, int y, int z);

/**
 * @brief Combine two 8 bit integers to a 16 bit one.
 *
 * @param higher higher byte for the 16 bit integer.
 * @param lower lower byte for the 16 bit integer.
 * @return uint16_t The combined 16 bit integer.
 */
uint16_t beemu_util_combine_8_to_16(uint8_t higher, uint8_t lower);

/**
 * @brief Split a 16 bit integer to two 8 bit integers.
 *
 * @param number Number to split into two.
 * @return BeemuByteTuple Return tuple.
 */
BeemuByteTuple beemu_util_split_16_to_8(uint16_t number);
#endif // BEEMU_INTERNALS_UTILITY_H