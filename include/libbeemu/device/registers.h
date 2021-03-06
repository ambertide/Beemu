#ifndef BEEMU_DEVICE_REGISTERS_H
#define BEEMU_DEVICE_REGISTERS_H
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Conditions that dictate when to jump.
 */
typedef enum BeemuJumpCondition
{
	BEEMU_JUMP_IF_NO_CONDITION,
	BEEMU_JUMP_IF_CARRY,
	BEEMU_JUMP_IF_NOT_CARRY,
	BEEMU_JUMP_IF_ZERO,
	BEEMU_JUMP_IF_NOT_ZERO
} BeemuJumpCondition;

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
 * @brief Get one of the 8 bit registers from its letters.
 *
 * @param letter Letter that refeers to the register.
 * @return BeemuRegister_8 Register refeered or A if not valid.
 */
BeemuRegister_8 beemu_get_register_from_letter_8(const char letter);

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
	BEEMU_REGISTER_AF
} BeemuRegister_16;

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
 * @brief Enums representing binary operations.
 *
 */
typedef enum BeemuOperation
{
	BEEMU_OP_ADD,
	BEEMU_OP_SUB,
	BEEMU_OP_AND,
	BEEMU_OP_OR,
	BEEMU_OP_CP,
	BEEMU_OP_XOR
} BeemuOperation;

/**
 * @brief Enums representing stack operation.
 */
typedef enum BeemuStackOperation
{
	BEEMU_SOP_POP,
	BEEMU_SOP_PUSH
} BeemuStackOperation;

/**
 * @brief Enums representing unary operations.
 *
 */
typedef enum BeemuUnaryOperation
{
	BEEMU_UOP_INC,
	BEEMU_UOP_DEC
} BeemuUnaryOperation;

/**
 * @brief Represents bit operations on a register.
 */
typedef enum BeemuBitOperation
{
	BEEMU_BIT_OP_BIT,
	BEEMU_BIT_OP_SET,
	BEEMU_BIT_OP_RES
} BeemuBitOperation;

/**
 * @brief Represents unary bit operations on a register.
 */
typedef enum BeemuUnaryBitOperation
{
	BEEMU_BIT_UOP_RLC,
	BEEMU_BIT_UOP_RRC,
	BEEMU_BIT_UOP_RL,
	BEEMU_BIT_UOP_RR,
	BEEMU_BIT_UOP_SLA,
	BEEMU_BIT_UOP_SRA,
	BEEMU_BIT_UOP_SWAP,
	BEEMU_BIT_UOP_SRL
} BeemuUnaryBitOperation;

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
 * @brief Clear all the flags to 0.
 *
 * @param registers BeemuRegisters object pointer.
 */
void beemu_registers_flags_clear(BeemuRegisters *registers);

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

/**
 * @brief Increment 16 bit register
 *
 * Increment a register.
 * @param registers BeemuRegisters object
 * @param register_name Name of the register to increment.
 */
void beemu_registers_increment_16(BeemuRegisters *registers, BeemuRegister_16 register_name);

/**
 * @brief Decrement a 16 bit register
 *
 * Decrement a register.
 * @param registers BeemuRegisters object pointer.
 * @param register_name Name of the register to decrement.
 */
void beemu_registers_decrement_16(BeemuRegisters *registers, BeemuRegister_16 register_name);

/**
 * @brief Increment 8 bit register
 *
 * Increment a register.
 * @param registers BeemuRegisters object
 * @param register_name Name of the register to increment.
 */
void beemu_registers_increment_8(BeemuRegisters *registers, BeemuRegister_8 register_name);

/**
 * @brief Perform operation on a register and write it to A.
 *
 * @param registers BeemuRegisters object pointer.
 * @param register Register to be added.
 * @param operation Operation to perform.
 * @param should_add_carry When set to true add the carry flag.
 */
void beemu_registers_arithmatic_8_register(BeemuRegisters *registers, BeemuRegister_8 register_, BeemuOperation operation, bool should_add_carry);

/**
 * @brief Perform operation on a register and write it to HL.
 *
 * @param registers BeemuRegisters object pointer.
 * @param register Register to be added.
 * @param operation Operation to perform.
 */
void beemu_registers_arithmatic_16_register(BeemuRegisters *registers, BeemuRegister_16 register_, BeemuOperation operation);

/**
 * @brief Perform operation on a constant and write it to A.
 *
 * @param registers BeemuRegisters object pointer.
 * @param value Value to perform operation on it.
 * @param operation Operation to perform.
 * @param should_add_carry When set to true add the carry flag.
 */
void beemu_registers_arithmatic_8_constant(BeemuRegisters *registers, uint8_t value, BeemuOperation operation, bool should_add_carry);

/**
 * @brief Perform operation on a constant and write it to HL.
 *
 * @param registers BeemuRegisters object pointer.
 * @param value Value to perform operation on it.
 * @param operation Operation to perform.
 */
void beemu_registers_arithmatic_16_constant(BeemuRegisters *registers, uint16_t value, BeemuOperation operation);

/**
 * @brief Perform a unary operation.
 *
 * Perform either a increment or a decrement operation
 * based on the operation value on a register and store
 * the result on the same register.
 * @param registers BeemuRegisters object pointer.
 * @param register_ Register to operate on.
 * @param operation Operation to perform.
 */
void beemu_registers_arithmatic_8_unary(BeemuRegisters *registers, BeemuRegister_8 register_, BeemuUnaryOperation operation);

/**
 * @brief Perform a unary operation on a 16-bit.
 *
 * Perform either a increment or a decrement operation
 * based on the operation value on a register and store
 * the result on the same register.
 * @param registers BeemuRegisters object pointer.
 * @param register_ Register to operate on.
 * @param operation Operation to perform.
 */
void beemu_registers_arithmatic_16_unary(BeemuRegisters *registers, BeemuRegister_16 register_, BeemuUnaryOperation operation);

/**
 * @brief Set flags based on registers.
 *
 * Determine and set the flags based on the previous and present
 * value of the registers.
 *
 * @param registers BeemuRegisters object.
 * @param previous_value Previous value of the register.
 * @param next_value Next value of the register.
 * @param after_add_carry First calculate carry, and then add the result to
 * next_value, and only after that determine and calculate the other flags.
 * @param operation Operation that was performed.
 * @param is_byte_length If set to true, perform on 8-bit operations.
 */
void beemu_registers_set_flags(BeemuRegisters *registers, uint16_t previous_value, uint16_t next_value, bool after_add_carry, BeemuOperation operation, bool is_byte_length);

/**
 * @brief Set the register to jump conditions.
 *
 * @param registers BeemuRegisters object.
 * @param condition Condition to jump on.
 * @param value Value to add to PC or to jump directly.
 * @param jump_directly If set to true, jump directly to the value
 * or the value of HL depending on the value of the next parameter.
 * @param jump_to_hl If set to true jump to the value of HL, ignoring the value.
 */
void beemu_registers_jump(BeemuRegisters *registers, BeemuJumpCondition condition, uint16_t value, bool jump_directly, bool jump_to_hl);

/**
 * @brief Rotate register A specifically.
 *
 * Set all three flags except C to 0, and then set C to the old
 * 7th bit.
 * @param registers BeemuRegisters object pointer.
 * @param rotate_right If set to true, rotate to right, if set
 * to false, rotate left.
 * @param through_c If set to true, put the previous value of C to
 * the most recent 0.
 */
void beemu_registers_rotate_A(BeemuRegisters *registers, bool rotate_right, bool through_c);

/**
 * @brief Complement the A register.
 *
 * @param registers BeemuRegisters object pointer.
 */
void beemu_registers_complement_A(BeemuRegisters *registers);

/**
 * @brief Convert the A register to BCD.
 *
 * Convert the A register to BCD.
 * @param registers BeemuRegisters object pointer.
 */
void beemu_registers_BCD(BeemuRegisters *registers);

/**
 * @brief Execute a bit operation on a register.
 *
 * @param registers BeemuRegisters object pointer.
 * @param operation Bit operation to execute,
 * BIT, SET or RES.
 * @param register_ Register to execute the operation on.
 * @param operand Operand for the operation
 */
void beemu_registers_execute_bit_operation(BeemuRegisters *registers, BeemuBitOperation operation, BeemuRegister_8 register_, uint8_t operand);

/**
 * @brief Execute a unary bit operation on a register.
 *
 * @param registers BeemuRegisters object pointer.
 * @param operation Bit operation to execute.
 * @param register_ Register to execute the operation in.
 */
void beemu_registers_execute_unary_bit_operation(BeemuRegisters *registers, BeemuUnaryBitOperation operation, BeemuRegister_8 register_);

#endif // BEEMU_DEVICE_REGISTERS_H
