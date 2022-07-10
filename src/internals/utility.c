#include <internals/utility.h>
#include <stdarg.h>

bool beemu_util_is_one_of(int element, int n, ...)
{
	va_list elements;
	va_start(elements, n);
	for (int i = 0; i < n; i++)
	{
		int e = va_arg(elements, int);
		if (element == e)
		{
			va_end(elements);
			return true;
		}
	}
	va_end(elements);
	return false;
}

inline bool beemu_util_is_one_of_two(int element, int x, int y)
{
	return element == x || element == y;
}

inline bool beemu_util_is_one_of_three(int element, int x, int y, int z)
{
	return element == x || element == y || element == z;
}

inline uint16_t beemu_util_combine_8_to_16(uint8_t higher, uint8_t lower)
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
