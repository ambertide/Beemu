#include <device/device.h>
#include <device/processor.h>
#include <stdlib.h>

BeemuDevice *beemu_device_new()
{
	BeemuProcessor *processor = beemu_processor_new();
	BeemuDevice *device = (BeemuDevice *)malloc(sizeof(BeemuDevice));
	device->processor = processor;
	return device;
}

void beemu_device_free(BeemuDevice *device)
{
	beemu_processor_free(device->processor);
	device->processor = 0;
	free(device);
}

void beemu_device_run(BeemuDevice *device)
{
	const uint8_t elapsed_cycle = beemu_processor_run(device->processor);
}
