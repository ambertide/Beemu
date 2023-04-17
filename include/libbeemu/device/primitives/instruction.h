#ifndef BEEMU_INSTRUCTION_H
#define BEEMU_INSTRUCTION_H

#include <stdint.h>
#include <stdbool.h>
#include "register.h"

typedef enum beemuInstructionType
{
	// Categorization based on https://gbdev.io/pandocs/CPU_Instruction_Set.html
	LOAD_8,
	LOAD_16,
	ARITHMATIC_8,
	ARITHMATIC_16,
	ROT_SHIFT,
	BITWISE,
	CPU_CONTROL,
	JUMP
} BeemuInstructionType;

/**
 * @brief Describes the type of a single param.
 *
 */
typedef enum beemuParamType
{
	MEMORY,
	REGISTER_8,
	REGISTER_16,
	UINT_8,
	UINT16,
	INT_8
} BeemuParamType;

/**
 * @brief Holds a single param.
 *
 * Holds a single param, its type
 * and its value.
 */
typedef struct BeemuParam
{
	/** Identifies whether to dereference before usage. */
	bool pointer;
	/** Type of a the variable. */
	BeemuParamType type;
	/** Actual value hold within. */
	union
	{
		uint16_t memory_address;
		uint16_t value;
		int8_t signed_value;
		BeemuRegister_8 register_;
		BeemuRegister_16 register_;
	} value;
} BeemuParam;

/**
 * @brief Parameters for the LOAD instructions.
 *
 */
typedef struct BeemuLoadParams
{
	/** Load from */
	BeemuParam source;
	/** Load to */
	BeemuParam dest;
} BeemuLoadParams;

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
	BEEMU_OP_XOR,
	BEEMU_POP,
	BEEMU_PUSH,
	BEEMU_INC,
	BEEMU_DEC
} BeemuOperation;

/** Params for arithmatic and logic ops. */
typedef struct BeemuArithmaticParams
{
	/** Specific arithmatic/logic operation to perform. */
	BeemuOperation operation;
	/** First operand as well as the destination register. */
	BeemuParam dest;
	/** Second operand, if exists. */
	BeemuParam source;
} BeemuArithmaticParams;

/**
 * @brief Specific operation subtype.
 *
 */
typedef enum BeemuRotShiftOp
{
	ROTATE,
	SHIFT_ARITHMATIC,
	SWAP,
	SHIFT_LOGICAL
} BeemuRotShiftOp;

/**
 * @brief Shift to left or right.
 *
 */
typedef enum BeemuRotShiftDirection
{
	LEFT,
	RIGHT,
} BeemuRotShiftDirection;

typedef struct BeemuRotShiftParams
{
	/** shift/rot through carry */
	bool through_carry;
	BeemuRotShiftOp operation;
	BeemuRotShiftDirection direction;
	/** either HL or 8 bit register. if HL, should be dereffed. */
	BeemuParam target;
} BeemuRotShiftParams;

/**
 * @brief Represents bit operations on a register.
 */
typedef enum BeemuBitOperation
{
	BEEMU_BIT_OP_BIT,
	BEEMU_BIT_OP_SET,
	BEEMU_BIT_OP_RES
} BeemuBitOperation;

typedef struct BeemuBitwiseParams
{
	BeemuBitOperation operation;
	/** Nth bit to set. */
	uint8_t bit_number;
	/** Target to act on, if HL, de reference, otherwise 8 bit. */
	BeemuParam target;
} BeemuBitwiseParams;

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

typedef enum BeemuJumpType
{
	JUMP,
	CALL,
	RET,
	RST
} BeemuJumpType;

/** Params used by jump instructions */
typedef struct BeemuJumpParams
{
	bool is_conditional;
	bool is_relative;
	bool enable_interrupts;
	BeemuJumpType type;
	BeemuJumpCondition condition;
	/** Extra param that may mean a lot of different things, depending on instruction. */
	BeemuParam param;

} BeemuJumpParams;

typedef enum BeemuSystemOperation
{
	XOR_CARRY_FLAG,
	SET_CARRY_FLAG,
	NOP,
	HALT,
	STOP,
	DISABLE_INTERRUPTS,
	ENABLE_INTERRUPTS
} BeemuSystemOperation;

/**
 * @brief Represents a single CPU instruction.
 *
 */
typedef struct BeemuInstruction
{
	BeemuInstructionType type;
	uint8_t duration_in_clock_cycles;
	uint8_t original_machine_code;
	union
	{
		BeemuLoadParams load_params;
		BeemuArithmaticParams arithmatic_params;
		BeemuRotShiftParams rot_shift_params;
		BeemuBitwiseParams bitwise_params;
		BeemuJumpParams jump_params;
		BeemuSystemOperation system_op;
	} params;
} BeemuInstruction;

#endif // BEEMU_INSTRUCTION_H
