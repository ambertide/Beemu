#ifndef BEEMU_REGISTERS_H
#define BEEMU_REGISTERS_H
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Represents 8 Bit registers.
 */
typedef enum BeemuRegister_8
{
	BEEMU_REGISTER_A,
	BEEMU_REGISTER_B,
	BEEMU_REGISTER_C,
	BEEMU_REGISTER_D,
	BEEMU_REGISTER_E,
	BEEMU_REGISTER_H,
	BEEMU_REGISTER_L
} BeemuRegister_8;

/**
 * @brief Represents the 16 bit registers
 *
 * These include the 16 bit combination registers
 * as well as pseudo-register M and special registers
 * SP and PC.
 */
typedef enum BeemuRegister_16
{
	BEEMU_REGISTER_BC,
	BEEMU_REGISTER_DE,
	BEEMU_REGISTER_HL,
	BEEMU_REGISTER_M,
	BEEMU_REGISTER_SP,
	BEEMU_REGISTER_PC,
} BeemuRegister_16;

/**
 * @brief Represents the flags.
 *
 */
typedef enum BeemuFlag
{
	BEEMU_FLAG_S = 7,  // Sign
	BEEMU_FLAG_Z = 6,  // Zero
	BEEMU_FLAG_AC = 4, // Auxiliary Carry
	BEEMU_FLAG_P = 2,  // Parity
	BEEMU_FLAG_C = 0   // Carry
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
 * @brief Read an 8-bit register.
 *
 * Read from A, B, C, D, E, H or L registers.
 * @param registers Pointer to registers
 * @param register_name Name of the register to read.
 * @return uint8_t register contents.
 */
uint8_t beemu_registers_read_8(BeemuRegisters *registers, BeemuRegister_8 register_name);

/**
 * @brief Write to an 8-bit register.
 *
 * Write to A, B, C, D, E, H or L registers.
 * @param registers BeemuRegisters object
 * @param register_name Name of the register to write.
 * @param value Value to write into the register.
 */
void beemu_registers_write_8(BeemuRegisters *registers, BeemuRegister_8 register_name, uint8_t value);

/**
 * @brief Read a 16-bit register
 *
 * Read from the BC, DE, HL, M, SP and PC
 * @param registers BeemuRegisters object
 * @param register_name Name of the register to read
 * @return uint16_t the value at the [pseudo-]register.
 */
uint16_t beemu_registers_read_16(BeemuRegisters *registers, BeemuRegister_16 register_name);

/**
 * @brief Write to a 16-bit register.
 *
 * Write to a 16-bit register.
 * @param registers BeemuRegisters object
 * @param register_name Name of the register to write.
 * @param value Value to write
 */
void beemu_registers_write_16(BeemuRegisters *registers, BeemuRegister_16 register_name, uint16_t value);

/**
 * @brief Read all of the flags as a single byte.
 *
 * @param registers BeemuRegisters object.
 * @return uint8_t the byte representing all the flags.
 */
uint8_t beemu_registers_flag_read_all(BeemuRegisters *registers);

/**
 * @brief Read a single flag.
 *
 * Read the value of a single flag padded with 0s on the
 * left.
 * @param registers BeemuRegisters object.
 * @param flag_name Name of the flag to read.
 * @return uint8_t Value of the single flag padded with 0s on the
 * left.
 */
uint8_t beemu_registers_flag_read(BeemuRegisters *registers, BeemuFlag flag_name);

/**
 * @brief Set the value of a flag.
 *
 * @param registers BeemuRegisters object.
 * @param flag_name Name of the flag to set
 * @param value If true, set the flag to 1, if false, set it to 0.
 */
void beemu_registers_flag_set(BeemuRegisters *registers, BeemuFlag flag_name, bool value);

/**
 * @brief Check if the flag is set to high.
 *
 * @param registers BeemuRegisters object.
 * @param flag_name Name of the flag to check.
 * @return true if the flag is set to one.
 * @return false if the flag is zero.
 */
bool beemu_registers_flag_is_high(BeemuRegisters *registers, BeemuFlag flag_name);

/**
 * @brief Return the PSW.
 *
 * Return the program status word (AF register)
 * @param registers BeemuRegisters object
 * @return uint16_t PSW value.
 */
uint16_t beemu_registers_read_psw(BeemuRegisters *registers);

#endif