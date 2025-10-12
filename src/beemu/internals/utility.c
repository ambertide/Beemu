#include <beemu/internals/utility.h>
#include <stdarg.h>

inline bool beemu_util_is_one_of_two(int element, int x, int y)
{
	return element == x || element == y;
}

inline bool beemu_util_is_one_of_three(int element, int x, int y, int z)
{
	return element == x || element == y || element == z;
}

inline uint16_t beemu_util_combine_8_to_16(const uint8_t higher, const uint8_t lower)
{
	return (((uint16_t)higher) << 8) | ((uint16_t)lower);
}

inline BeemuByteTuple beemu_util_split_16_to_8(uint16_t number)
{
	const BeemuByteTuple decomposition = {
		((uint8_t)((number & 0xFF00) >> 8)),
		((uint8_t)((number & 0x00FF)))};
	return decomposition;
}

inline uint8_t beemu_util_combine_4_to_8(uint8_t higher, uint8_t lower)
{
	return (higher << 4) | lower;
}

inline BeemuByteTuple beemu_util_split_8_to_4(uint8_t number)
{
	const BeemuByteTuple ret = {
		(number & 0xF0) >> 4,
		number & 0x0F};
	return ret;
}

inline uint8_t beemu_util_swap_nibbles(uint8_t number)
{
	const BeemuByteTuple tuple = beemu_util_split_8_to_4(number);
	return beemu_util_combine_4_to_8(tuple.second, tuple.first);
}

BeemuByteTuple beemu_util_rotate_or_shift(uint8_t value, uint8_t extra_bit, bool rotate, bool through_extra_bit, BeemuRotationDirection direction, bool keep_msb)
{
	uint8_t new_value = (value << 1);
	uint8_t new_extra_bit = value & 0x01;
	if (through_extra_bit && rotate)
	{
		new_value |= extra_bit;
	}
	else if (rotate)
	{
		// On rotate left
		new_value |= (value >> 7);
	}
	if (direction == BEEMU_ROTATION_DIRECTION_RIGHT)
	{
		new_value >>= 2;
		new_extra_bit = (0x80 & value) >> 7;
		if (through_extra_bit && rotate)
		{
			new_value |= (extra_bit << 7);
		}
		else if (rotate)
		{
			new_value |= (value << 7);
		}
	}
	if (keep_msb)
	{
		new_value = new_value | (value & 0x80);
	}
	BeemuByteTuple tuple = {new_value, new_extra_bit};
	return tuple;
}
