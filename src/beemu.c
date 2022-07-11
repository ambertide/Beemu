#include <stdio.h>
#include <device/processor.h>
#include <version.h>

int main()
{
	BeemuProcessor *processor = beemu_processor_new();
	printf("%s, V%d.%d\n", "Welcome to Beemu 🐝", Beemu_VERSION_MAJOR, Beemu_VERSION_MINOR);
}
