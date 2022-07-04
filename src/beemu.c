#include <stdio.h>
#include <device/device.h>
#include <version.h>

int main()
{
	BeemuDevice *device = beemu_device_new();
	printf("%s, V%d.%d\n", "Welcome to Beemu 🐝", Beemu_VERSION_MAJOR, Beemu_VERSION_MINOR);
}
