#ifndef BEEMU_INTERNALS_UTILITY_H
#define BEEMU_INTERNALS_UTILITY_H

#include <stdbool.h>

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

#endif // BEEMU_INTERNALS_UTILITY_H