#ifndef BEEMU_DEVICE_H
#define BEEMU_DEVICE_H
#include <stdint.h>
#include <stdbool.h>
#include "registers.h"
#include "memory.h"

static const BeemuRegister_8 ORDERED_REGISTER_NAMES[8] = {BEEMU_REGISTER_B,
														  BEEMU_REGISTER_C,
														  BEEMU_REGISTER_D,
														  BEEMU_REGISTER_E,
														  BEEMU_REGISTER_H,
														  BEEMU_REGISTER_L,
														  BEEMU_REGISTER_A,
														  BEEMU_REGISTER_A};
static const BeemuRegister_16 ORDERED_REGISTER_NAMES_16[4] = {
	BEEMU_REGISTER_BC,
	BEEMU_REGISTER_DE,
	BEEMU_REGISTER_HL,
	BEEMU_REGISTER_SP};

static const BeemuRegister_16 ORDERED_REGISTER_STACK_NAMES_16[4] = {
	BEEMU_REGISTER_BC,
	BEEMU_REGISTER_DE,
	BEEMU_REGISTER_HL,
	BEEMU_REGISTER_AF};

/**
 * @brief Used to distinguish symmetrical registers such as used by
 * 03 Block's LD, INC and DEC.
 */
static const BeemuRegister_8 SYMMETRIC_REGISTERS[4][2] = {{BEEMU_REGISTER_B, BEEMU_REGISTER_C}, {BEEMU_REGISTER_D, BEEMU_REGISTER_E}, {BEEMU_REGISTER_H, BEEMU_REGISTER_L}, {BEEMU_REGISTER_A, BEEMU_REGISTER_A}};

/**
 * @brief Size of the chip memory.
 */
static const int BEEMU_DEVICE_MEMORY_SIZE = 64000;
/**
 * @brief Location of the ROM in the main memory.
 */
static const int BEEMU_DEVICE_MEMORY_ROM_LOCATION;

/**
 * @brief Describes the device state.
 *
 */
typedef enum BeemuDeviceState
{
	BEEMU_DEVICE_NORMAL,
	BEEMU_DEVICE_HALT,
	BEEMU_DEVICE_STOP,
	BEEMU_DEVICE_AWAITING_INTERRUPT_DISABLE,
	BEEMU_DEVICE_AWAITING_INTERRUPT_ENABLE
} BeemuDeviceState;

typedef struct BeemuDevice
{
	BeemuRegisters *registers;
	BeemuMemory *memory;
	BeemuDeviceState device_state;
	bool interrupts_enabled;
	/**
	 * @brief The currently executed instruction.
	 */
	struct
	{
		uint8_t instruction;
		uint8_t first_nibble;
		uint8_t second_nibble;
	} current_instruction;
	union data
	{
		uint8_t data_8;
		uint16_t data_16;
	} data;
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

/**
 * @brief Get the state of the device.
 *
 * Get the current state of the device.
 * @param device BeemuDevice object pointer.
 * @return BeemuDeviceState
 */
BeemuDeviceState beemu_device_get_device_state(BeemuDevice *device);

/**
 * @brief Set the state of the device.
 *
 * Set the state of the device.
 * @param device BeemuDevice object pointer.
 * @param state The new state of the device.
 */
void beemu_device_set_state(BeemuDevice *device, BeemuDeviceState state);
#endif // BEEMU_DEVICE_H
