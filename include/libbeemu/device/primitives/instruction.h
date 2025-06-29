#ifndef BEEMU_INSTRUCTION_H
#define BEEMU_INSTRUCTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "register.h"
#include <stdbool.h>
#include <stdint.h>

	typedef enum beemuInstructionType {
		// Categorization based on https://gbdev.io/pandocs/CPU_Instruction_Set.html
		BEEMU_INSTRUCTION_TYPE_LOAD,
		BEEMU_INSTRUCTION_TYPE_ARITHMATIC,
		BEEMU_INSTRUCTION_TYPE_ROT_SHIFT,
		BEEMU_INSTRUCTION_TYPE_BITWISE,
		BEEMU_INSTRUCTION_TYPE_CPU_CONTROL,
		BEEMU_INSTRUCTION_TYPE_JUMP
	} BeemuInstructionType;

	/**
	 * @brief Describes the type of a single param.
	 *
	 */
	typedef enum BeemuParamType {
		BEEMU_PARAM_TYPE_REGISTER_8,
		BEEMU_PARAM_TYPE_REGISTER_16,
		BEEMU_PARAM_TYPE_UINT_8,
		BEEMU_PARAM_TYPE_UINT16,
		BEEMU_PARAM_TYPE_INT_8
	} BeemuParamType;

	typedef enum BeemuWriteLength {
		BEEMU_WRITE_LENGTH_8,
		BEEMU_WRITE_LENGTH_16
	} BeemuWriteLength;

	/**
	 * @brief Holds a single param.
	 *
	 * Holds a single param, its type
	 * and its value.
	 */
	typedef struct BeemuParam {
		/** Identifies whether to dereference before usage. */
		bool pointer;
		/** Type of a the variable. */
		BeemuParamType type;
		/** Actual value hold within. */
		union {
			uint16_t value;
			int8_t signed_value;
			BeemuRegister_8 register_8;
			BeemuRegister_16 register_16;
		} value;
	} BeemuParam;

	/**
	 * @brief Extra operation to perform immediately following a load operation.
	 *
	 */
	typedef enum BeemuPostLoadOperation {
		BEEMU_POST_LOAD_NOP,
		BEEMU_POST_LOAD_INCREMENT_INDIRECT_SOURCE,
		BEEMU_POST_LOAD_DECREMENT_INDIRECT_SOURCE,
		BEEMU_POST_LOAD_DECREMENT_INDIRECT_DESTINATION,
		BEEMU_POST_LOAD_INCREMENT_INDIRECT_DESTINATION,
		/** Parse a signed payload from the instruction and add
		 * it on top of the newly loaded destination. */
		BEEMU_POST_LOAD_SIGNED_PAYLOAD_SUM
	} BeemuPostLoadOperation;

	/**
	 * @brief Parameters for the LOAD instructions.
	 *
	 */
	typedef struct BeemuLoadParams {
		/** Load from */
		BeemuParam source;
		/** Load to */
		BeemuParam dest;
		/**
		 * @brief Certain instructions perform an additional op immediately
		 * after loading.
		 */
		BeemuPostLoadOperation postLoadOperation;
		/**
		 * Possibly used when the post load operation is BEEMU_POST_LOAD_SIGNED_PAYLOAD
		 * or some other value that requires a third value.
		 */
		BeemuParam auxPostLoadParameter;
	} BeemuLoadParams;

	/**
	 * @brief Enums representing binary operations.
	 *
	 */
	typedef enum BeemuOperation {
		BEEMU_OP_ADD,
		BEEMU_OP_ADC,
		BEEMU_OP_SUB,
		BEEMU_OP_SBC,
		BEEMU_OP_AND,
		BEEMU_OP_OR,
		BEEMU_OP_CP,
		BEEMU_OP_XOR,
		BEEMU_OP_DAA,
		BEEMU_OP_CPL,
		BEEMU_OP_SCF,
		BEEMU_OP_CCF
	} BeemuOperation;

	/** Params for arithmatic and logic ops. */
	typedef struct BeemuArithmaticParams {
		/** Specific arithmatic/logic operation to perform. */
		BeemuOperation operation;
		/** First operand as well as the destination register. */
		BeemuParam dest_or_first;
		/** Second operand and (possibly one of) the source register(s). */
		BeemuParam source_or_second;
	} BeemuArithmaticParams;

	/**
	 * @brief Specific operation subtype.
	 *
	 */
	typedef enum BeemuRotShiftOp {
		BEEMU_ROTATE_OP,
		BEEMU_SHIFT_ARITHMATIC_OP,
		BEEMU_SWAP_OP,
		BEEMU_SHIFT_LOGICAL_OP
	} BeemuRotShiftOp;

	/**
	 * @brief Shift to left or right.
	 *
	 */
	typedef enum BeemuRotShiftDirection {
		BEEMU_LEFT_DIRECTION,
		BEEMU_RIGHT_DIRECTION,
	} BeemuRotShiftDirection;

	typedef struct BeemuRotShiftParams {
		/** shift/rot through carry */
		bool through_carry;
		bool set_flags_to_zero;
		BeemuRotShiftOp operation;
		BeemuRotShiftDirection direction;
		/** either HL or 8 bit register. if HL, should be dereffed. */
		BeemuParam target;
	} BeemuRotShiftParams;

	/**
	 * @brief Represents bit operations on a register.
	 */
	typedef enum BeemuBitOperation {
		BEEMU_BIT_OP_BIT,
		BEEMU_BIT_OP_SET,
		BEEMU_BIT_OP_RES
	} BeemuBitOperation;

	typedef struct BeemuBitwiseParams {
		BeemuBitOperation operation;
		/** Nth bit to set. */
		uint8_t bit_number;
		/** Target to act on, if HL, de reference, otherwise 8 bit. */
		BeemuParam target;
	} BeemuBitwiseParams;

	/**
	 * @brief Conditions that dictate when to jump.
	 */
	typedef enum BeemuJumpCondition {
		BEEMU_JUMP_IF_NO_CONDITION,
		BEEMU_JUMP_IF_CARRY,
		BEEMU_JUMP_IF_NOT_CARRY,
		BEEMU_JUMP_IF_ZERO,
		BEEMU_JUMP_IF_NOT_ZERO
	} BeemuJumpCondition;

	typedef enum BeemuJumpType {
		BEEMU_JUMP_TYPE_JUMP,
		BEEMU_JUMP_TYPE_CALL,
		BEEMU_JUMP_TYPE_RET,
		BEEMU_JUMP_TYPE_RST
	} BeemuJumpType;

	/** Params used by jump instructions */
	typedef struct BeemuJumpParams {
		bool is_conditional;
		bool is_relative;
		bool enable_interrupts;
		BeemuJumpType type;
		BeemuJumpCondition condition;
		/** Extra param that may mean a lot of different things, depending on instruction. */
		BeemuParam param;

	} BeemuJumpParams;

	typedef enum BeemuSystemOperation {
		BEEMU_CPU_OP_NOP,
		BEEMU_CPU_OP_HALT,
		BEEMU_CPU_OP_STOP,
		BEEMU_CPU_OP_DISABLE_INTERRUPTS,
		BEEMU_CPU_OP_ENABLE_INTERRUPTS
	} BeemuSystemOperation;

	/**
	 * @brief Represents a single CPU instruction.
	 *
	 */
	typedef struct BeemuInstruction {
		BeemuInstructionType type;
		uint8_t duration_in_clock_cycles;
		uint32_t original_machine_code;
		uint8_t byte_length;
		union {
			BeemuLoadParams load_params;
			BeemuArithmaticParams arithmatic_params;
			BeemuRotShiftParams rot_shift_params;
			BeemuBitwiseParams bitwise_params;
			BeemuJumpParams jump_params;
			BeemuSystemOperation system_op;
		} params;
	} BeemuInstruction;

#ifdef __cplusplus
}
#endif

#endif // BEEMU_INSTRUCTION_H"
