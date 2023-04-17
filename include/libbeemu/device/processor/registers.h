#ifndef BEEMU_DEVICE_REGISTERS_H
#define BEEMU_DEVICE_REGISTERS_H
#include <stdint.h>
#include <stdbool.h>
#include "../primitives/register.h"

/**
 * @brief Represents the flags.
 *
 */
typedef enum BeemuFlag
{
	BEEMU_FLAG_Z = 7, // Zero
	BEEMU_FLAG_N = 6, // Auxiliary Carry
	BEEMU_FLAG_H = 5, // Parity
	BEEMU_FLAG_C = 4  // Carry
} BeemuFlag;

/**
 * @brief Struct holding registers and flags.
 *
 */
typedef struct BeemuRegisters
{
	uint8_t registers[7];
	uint8_t flags;
	uint16_t stack_pointer;
	uint16_t program_counter;
	/**
	 * Special pseudoregister holding the
	 * HL dereferenced.
	 */
	uint8_t m_register;
} BeemuRegisters;

/**
 * @brief Create a new BeemuRegisters object.
 *
 * Create a new BeemuRegisters object, initialise
 * everything to zero.
 * @return BeemuRegisters* pointer to the newly created
 * BeemuRegisters object.
 */
BeemuRegisters *beemu_registers_new(void);

/**
 * @brief Free the BeemuRegisters object.
 *
 * @param registers Object to free.
 */
void beemu_registers_free(BeemuRegisters *registers);

/**
 * @brief Read the value from a single register
 *
 * @param registers Registers pointer
 * @param register_ Register value to read from.
 * @return uint16_t Value.
 */
uint16_t beemu_read_register_value(BeemuRegisters *registers, BeemuRegister register_);

/**
 * @brief Write to a register, a value.
 *
 * @param registers Registers file pointer.
 * @param register_ Register to write to.
 * @param value Value to write.
 */
void beemu_write_register_value(BeemuRegisters *registers, BeemuRegister register_, uint16_t value);

#endif // BEEMU_DEVICE_REGISTERS_H
