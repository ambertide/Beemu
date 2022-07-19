#ifndef BEEMU_DEVICE_DEVICE_H
#define BEEMU_DEVICE_DEVICE_H

#include "processor.h"

/**
 * @brief Device object that keeps everything contained.
 *
 */
typedef struct BeemuDevice
{
	BeemuProcessor *processor;
} BeemuDevice;

/**
 * @brief Initialize a new BeemuDevice.
 *
 * @return BeemuDevice* Pointer to the new object.
 */
BeemuDevice *beemu_device_new();

/**
 * @brief Free the pointer.
 *
 * @param device BeemuDevice to free.
 */
void beemu_device_free(BeemuDevice *device);

/**
 * @brief Run for one instruction.
 *
 * @param device
 */
void beemu_device_run(BeemuDevice *device);

#endif // BEEMU_DEVICE_DEVICE_H