#ifndef BEEMU_INTERNALS_UTILITY_H
#define BEEMU_INTERNALS_UTILITY_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct BeemuByteTuple
	{
		uint8_t first;
		uint8_t second;
	} BeemuByteTuple;

	typedef enum BeemuRotationDirection
	{
		BEEMU_ROTATION_DIRECTION_LEFT,
		BEEMU_ROTATION_DIRECTION_RIGHT
	} BeemuRotationDirection;

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

	/**
	 * @brief Combine two 4 bit integers to a 8 bit one.
	 *
	 * @param higher higher nibble for the 8 bit integer.
	 * @param lower lower nibble for the 8 bit integer.
	 * @return uint16_t The combined 8 bit integer.
	 */
	uint8_t beemu_util_combine_4_to_8(uint8_t higher, uint8_t lower);

	/**
	 * @brief Split a 8 bit integer to two 4 bit integers.
	 *
	 * @param number Number to split into two.
	 * @return BeemuByteTuple Return tuple.
	 */
	BeemuByteTuple beemu_util_split_8_to_4(uint8_t number);

	/**
	 * @brief Swap the nibbles of a byte.
	 *
	 * @param number Number to swap the bytes of.
	 * @return uint8_t The byte with swapped nibbles.
	 */
	uint8_t beemu_util_swap_nibbles(uint8_t number);

	/**
	 * @brief Rotate or shift an 8 bit value.
	 *
	 * @param value Value to shift or rotate.
	 * @param extra_bit An extra bit to include.
	 * @param rotate If set to true, rotate, otherwise shift.
	 * @param through_extra_bit Rotate through the extra bit if set to true.
	 * @param direction Direction of the rotation/shift
	 * @param keep_msb If set to true, keep the MSB the same.
	 * @return uint8_t The rotated/shifted value and the new extra bit value.
	 */
	BeemuByteTuple beemu_util_rotate_or_shift(uint8_t value, uint8_t extra_bit, bool rotate, bool through_extra_bit, BeemuRotationDirection direction, bool keep_msb);

	// Current releases of msvc does not support stdckdint
	// while clang and gcc do, therefore we need to seperate these two.
	// For msvc we must use an internal windows header which is activated
	// via a macro declaration.
	#ifdef _WIN32
	#ifndef ENABLE_INTSAFE_SIGNED_FUNCTIONS
	#define ENABLE_INTSAFE_SIGNED_FUNCTIONS
	#include <intsafe.h>
	#define BEEMU_CKD_ADD(result, a, b) IntAdd(a, b, result)
	#endif
	#else
	#include <stdckdint.h>
	#define BEEMU_CKD_ADD(result, a, b) ckd_add(result, a, b)
	#endif

#ifdef __cplusplus
}
#endif
#endif // BEEMU_INTERNALS_UTILITY_H
