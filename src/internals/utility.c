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
