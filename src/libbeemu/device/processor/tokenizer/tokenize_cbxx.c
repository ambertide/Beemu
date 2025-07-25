#include "tokenize_cbxx.h"
#include "tokenize_common.h"

/** CB Prefixed COMMON PARSING FUNCTIONS */

/**
 * @brief Tokenize the target instruction for a CB Prefixed instruction.
 *
 * CB Prefixed instructions always target a register (or the pseudoregister (HL)) and
 * which one they target depend almost entirely on the most significant byte,
 * (or specificially the last three bits of the MSB since they repeat every 8 bits.)
 *
 * @param instruction Partially constructed token.
 * @return BeemuParam Param to put to the target field.
 */
BeemuParam cb_tokenize_target(BeemuInstruction *instruction)
{
	uint8_t arithmatic_differentiator = instruction->original_machine_code & 0x00FF;
	uint8_t register_differentiator = arithmatic_differentiator & 0x07;
	BeemuParam target;
	tokenize_register_param_with_index(&target, register_differentiator);
	return target;
}

/** CB Prefixed ROT/SHIFT OPERATIONS */

/**
 * @brief Determine rot shift op subtype.
 *
 * @param instr partially built instruction ptr.
 */
void cb_rot_shift_determine_subtype(BeemuInstruction *inst)
{
	uint8_t arithmatic_differentiator = inst->original_machine_code & 0x00FF;
	const static BeemuRotShiftOp subtypes_by_second_bit[] = {
		BEEMU_ROTATE_OP,
		BEEMU_ROTATE_OP,
		BEEMU_SHIFT_ARITHMATIC_OP,
		BEEMU_SWAP_OP};
	inst->type = BEEMU_INSTRUCTION_TYPE_ROT_SHIFT;

	if (arithmatic_differentiator <= 0x3F && arithmatic_differentiator >= 0x38)
	{
		inst->type = BEEMU_INSTRUCTION_TYPE_ROT_SHIFT;
		inst->params.rot_shift_params.operation = BEEMU_SHIFT_LOGICAL_OP;
	}
	else
	{
		inst->params.rot_shift_params.operation = subtypes_by_second_bit[arithmatic_differentiator >> 4];
	}
}

/**
 * @brief Determine rot shift parameters
 *
 * This determines the direction of the rot shift, carry flag status etc.
 * Basically everything EXCEPT the type/subtype.
 *
 */
void cb_rot_shift_determine_params(BeemuInstruction *instruction)
{
	BeemuRotShiftOp subtype = instruction->params.rot_shift_params.operation;
	uint8_t arithmatic_differentiator = instruction->original_machine_code & 0x00FF;
	uint8_t rotshift_differentiator = arithmatic_differentiator >> 4;
	// C flag affects are true for the 0xCB00-CB10 RLA/RRA bloc, 0xCB20-CB30 SLA/SRA bloc and the 0xCB38-CB40 SRL bloc.
	instruction->params.rot_shift_params.through_carry = rotshift_differentiator == 0x0 || rotshift_differentiator == 0x2 || arithmatic_differentiator >= 0x38;
	// Theoratically the below is false since SWAP should not have a direction, but BEEMU_LEFT_DIRECTION is 0 in memory
	// so it *should* be fine.
	instruction->params.rot_shift_params.direction = rotshift_differentiator <= 0x8 ? BEEMU_LEFT_DIRECTION : BEEMU_RIGHT_DIRECTION;
	uint8_t register_differentiator = arithmatic_differentiator & 0x07;
	instruction->params.rot_shift_params.target = cb_tokenize_target(instruction);
	// This is only used for special rot shifts in the main space.
	instruction->params.rot_shift_params.set_flags_to_zero = instruction->byte_length == 1;
}

/** CB-PREFIXED BITWISE OPERATIONS */

/**
 * @brief Determine the subtype of a bitwise inst
 *
 * This is one of the BIT, SET or RES instructions.
 *
 * @param inst partially constructed instruction token.
 */
void cb_bitwise_determine_subtype(BeemuInstruction *inst)
{
	static const BeemuBitOperation operations[] = {
		BEEMU_BIT_OP_BIT,
		BEEMU_BIT_OP_RES,
		BEEMU_BIT_OP_SET};
	// You can determine the operation using 4th most significant
	// nibble and than negating one from it, so for instance, starting
	// from the second most significant byte:
	// 0x4 0100 xxxx > 6 = 01 - 01 = 0 BIT
	// 0x5 0101 xxxx > 6 = 01 - 01 = 0 BIT
	// 0x6 0110 xxxx > 6 = 01 - 01 = 0 BIT
	// 0x7 0111 xxxx > 6 = 01 - 01 = 0 BIT
	// 0x8 1000 xxxx > 6 = 10 - 01 = 1 RES
	// 0x9 1001 xxxx > 6 = 10 - 01 = 1 RES
	// 0xA 1010 xxxx > 6 = 10 - 01 = 1 RES
	// 0xB 1011 xxxx > 6 = 10 - 01 = 1 RES
	// 0xC 1100 xxxx > 6 = 11 - 01 = 2 SET
	// 0xD 1101 xxxx > 6 = 11 - 01 = 2 SET
	// 0xE 1110 xxxx > 6 = 11 - 01 = 2 SET
	// 0xF 1111 xxxx > 6 = 11 - 01 = 2 SET
	// Mask the fourth most significant nibble
	// shift it rightwards to be at the start and
	// then subtract one from it.
	const uint8_t op_differentiator = ((inst->original_machine_code & 0x00C0) >> 6) - 1;
	inst->params.bitwise_params.operation = operations[op_differentiator];
}

/**
 * @brief Determine the parameters of a BIT/RES/SET instruction.
 *
 * Determines the bit number and the register.
 * @param inst
 */
void cb_bitwise_determine_params(BeemuInstruction *inst)
{
	inst->params.bitwise_params.target = cb_tokenize_target(inst);
	// We are taking this as our target as this bitmask matches the pattern of incrementing
	// the bit index every once in 8 instructions AND resetting every once in 64 instructions
	// or so.
	inst->params.bitwise_params.bit_number = ((inst->original_machine_code & 0b111000) >> 3);
}

/** COMMON CB Prefixed OPERATIONS */

/**
 * @brief Determine cb prefixed operation and suboperations.
 *
 * @param inst partially built instruction ptr.
 */
void cb_determine_type(BeemuInstruction *inst)
{
	uint8_t arithmatic_differentiator = inst->original_machine_code & 0x00FF;

	if (arithmatic_differentiator < 0x40)
	{
		inst->type = BEEMU_INSTRUCTION_TYPE_ROT_SHIFT;
		cb_rot_shift_determine_subtype(inst);
	}
	else
	{
		inst->type = BEEMU_INSTRUCTION_TYPE_BITWISE;
		cb_bitwise_determine_subtype(inst);
	}
}

/**
 * @brief Determine parameters for a cb prefixed instruction.
 *
 * @param instruction
 */
void cb_determine_params(BeemuInstruction *instruction)
{
	if (instruction->type == BEEMU_INSTRUCTION_TYPE_ROT_SHIFT)
	{
		cb_rot_shift_determine_params(instruction);
	}
	else if (instruction->type == BEEMU_INSTRUCTION_TYPE_BITWISE)
	{
		cb_bitwise_determine_params(instruction);
	}
}

/**
 * @brief Determine the time it takes for an instruction execution to complete for cb prefixed ones.
 *
 * @param instruction
 */
void cb_determine_clock_cycles(BeemuInstruction *instruction)
{
	uint8_t clock_cycle_differentiator = instruction->original_machine_code & 0x07;
	if (clock_cycle_differentiator == 0x06)
	{
		// (HL) access instructions take longer.
		uint8_t secondary_clock_cycle_differentiator = (instruction->original_machine_code & 0xFF) >> 4;
		// BIT block between 0xCB40-0xCB70's (HL) accessing instructions execute in 3 clock cycles
		// rather than 4.
		if (secondary_clock_cycle_differentiator >= 0x04 && secondary_clock_cycle_differentiator <= 0x07)
		{
			instruction->duration_in_clock_cycles = 3;
		}
		else
		{
			instruction->duration_in_clock_cycles = 4;
		}
	}
	else
	{
		// Other cb prefixed instructions take 2 cycles always.
		instruction->duration_in_clock_cycles = 2;
	}
}

void tokenize_single_byte_rotates(BeemuInstruction *instruction)
{
	instruction->type = BEEMU_INSTRUCTION_TYPE_ROT_SHIFT;
	instruction->duration_in_clock_cycles = 1;
	instruction->params.rot_shift_params.set_flags_to_zero = true;
	instruction->params.rot_shift_params.direction = (instruction->original_machine_code & 0xF) == 0xF ? BEEMU_RIGHT_DIRECTION : BEEMU_LEFT_DIRECTION;
	instruction->params.rot_shift_params.operation = BEEMU_ROTATE_OP;
	instruction->params.rot_shift_params.through_carry = (instruction->original_machine_code & 0xF0) == 0x00;
	tokenize_register_param_with_index(
		&instruction->params.rot_shift_params.target,
		7
	);
}

void tokenize_cbxx(BeemuInstruction *instruction)
{
	if (instruction->byte_length == 1) {
		// RRA, RLA, RRCA, RLCA
		tokenize_single_byte_rotates(instruction);
	} else {
		// now, a very cool fact of life is that we can actually lowkey
		// determine the instruction parameters seperately.
		cb_determine_type(instruction);
		cb_determine_params(instruction);
		cb_determine_clock_cycles(instruction);
	}
}
