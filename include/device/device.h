#ifndef BEEMU_DEVICE_H
#define BEEMU_DEVICE_H
#include <stdint.h>
#include <stdbool.h>
#include "registers.h"
#include "memory.h"

/**
 * @brief Size of the chip memory.
 */
static const int BEEMU_DEVICE_MEMORY_SIZE = 64000;
/**
 * @brief Location of the ROM in the main memory.
 */
static const int BEEMU_DEVICE_MEMORY_ROM_LOCATION;

typedef struct BeemuDevice
{
	BeemuRegisters *registers;
	BeemuMemory *memory;
} BeemuDevice;

/**
 * @brief Create a new BeemuDevice instance.
 *
 * Create a new BeemuDevice and return its pointer.
 * @return BeemuDevice* pointer to the newly created device object.
 */
BeemuDevice *beemu_device_new(void);

/**
 * @brief Free the previously created device.
 *
 * @param device Device to free.
 */
void beemu_device_free(BeemuDevice *device);

/**
 * @brief Load ROM data to device.
 *
 * Load a GameBoy ROM to the device memory.
 * @param device BeemuDevice instance to load the ROM.
 * @param rom ROM data to be loaded.
 * @return bool Whether or not the load succeeded.
 */
bool beemu_device_load(BeemuDevice *device, uint8_t *rom);

/**
 * @brief Run the loaded ROM.
 *
 * Run the data loaded at the ROM section of the memory.
 * @param device BeemuDevice object pointer.
 */
void beemu_device_run(BeemuDevice *device);

#endif // BEEMU_DEVICE_H
