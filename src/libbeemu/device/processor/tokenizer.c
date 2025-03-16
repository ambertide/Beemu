#include <stdlib.h>
#include <libbeemu/device/processor/tokenizer.h>
#include <libbeemu/device/processor/processor.h>
#include <libbeemu/device/memory.h>
#include <libbeemu/internals/utility.h>

/**
 * @brief Determine rot shift op subtype.
 *
 * @param instr partially built instruction ptr.
 */
void bt16_rot_shift_determine_subtype(BeemuInstruction *inst)
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
		inst->params.rot_shift_params.operation = subtypes_by_second_bit[arithmatic_differentiator >> 8];
	}
}

/**
 * @brief Determine rot shift parameters
 *
 * This determines the direction of the rot shift, carry flag status etc.
 * Basically everything EXCEPT the type/subtype.
 *
 */
void bt16_rot_shift_determine_params(BeemuInstruction *instruction)
{
	BeemuRotShiftOp subtype = instruction->params.rot_shift_params.operation;
	uint8_t arithmatic_differentiator = instruction->original_machine_code & 0x00FF;
	uint8_t rotshift_differentiator = arithmatic_differentiator >> 8;
	// C flag affects are true for the 0xCB00-CB10 RLA/RRA bloc, 0xCB20-CB30 SLA/SRA bloc and the 0xCB38-CB40 SRL bloc.
	instruction->params.rot_shift_params.through_carry = rotshift_differentiator == 0x0 || rotshift_differentiator == 0x2 || arithmatic_differentiator >= 0x38;
	// Theoratically the below is false since SWAP should not have a direction, but BEMU_LEFT_DIRECTION is 0 in memory
	// so it *should* be fine.
	instruction->params.rot_shift_params.direction = rotshift_differentiator <= 0x8 ? BEMU_LEFT_DIRECTION : BEEMU_RIGHT_DIRECTION;
	uint8_t register_differentiator = arithmatic_differentiator & 0x07;
	if (register_differentiator == 0x06)
	{
		// This is the case for (HL), the dereferenced HL pointer to the memory.
		instruction->params.rot_shift_params.target.pointer = true;
		instruction->params.rot_shift_params.target.type = BEEMU_PARAM_TYPE_REGISTER_16;
		instruction->params.rot_shift_params.target.value.register_16 = BEEMU_REGISTER_HL;
	}
	else
	{
		const static BeemuRegister_8 registers[] = {
			BEEMU_REGISTER_B,
			BEEMU_REGISTER_C,
			BEEMU_REGISTER_D,
			BEEMU_REGISTER_E,
			BEEMU_REGISTER_H,
			BEEMU_REGISTER_L,
			BEEMU_REGISTER_A, // Technically the code should NEVER hit here.
			BEEMU_REGISTER_A};
		// Otherwise all 0xCB00-40 block acts on 8 bit register values.
		instruction->params.rot_shift_params.target.pointer = false; // Not really necessary btw.
		instruction->params.rot_shift_params.target.type = BEEMU_PARAM_TYPE_REGISTER_8;
		instruction->params.rot_shift_params.target.value.register_8 = registers[register_differentiator];
	}
}

/**
 * @brief Determine 16 bit operation and suboperations.
 *
 * @param inst partially built instruction ptr.
 */
void bt16_determine_type(BeemuInstruction *inst)
{
	uint8_t arithmatic_differentiator = inst->original_machine_code & 0x00FF;

	if (arithmatic_differentiator < 0x40)
	{
		inst->type = BEEMU_INSTRUCTION_TYPE_ROT_SHIFT;
		bt16_rot_shift_determine_subtype(inst);
	}
}

/**
 * @brief Determine parameters for a 16 bit instruction.
 *
 * @param instruction
 */
void bt16_determine_params(BeemuInstruction *instruction)
{
	if (instruction->type == BEEMU_INSTRUCTION_TYPE_ROT_SHIFT)
	{
		bt16_rot_shift_determine_params(instruction);
	}
}

/**
 * @brief Determine the time it takes for an instruction execution to complete for 16 bit ones.
 *
 * @param instruction
 */
void bt16_determine_clock_cycles(BeemuInstruction *instruction)
{
	uint8_t clock_cycle_differentiator = instruction->original_machine_code & 0x07;
	if (clock_cycle_differentiator == 0x06)
	{
		// (HL) access instructions take longer.
		uint8_t secondary_clock_cycle_differentiator = (instruction->original_machine_code & 0xFF) > 8;
		// The BIT block is 4, others are 4 for (HL acccess)
		instruction->duration_in_clock_cycles = secondary_clock_cycle_differentiator - 0x07 <= 3 ? 3 : 4;
	}
	else
	{
		// Other 16 bit instructions take 2 cycles always.
		instruction->duration_in_clock_cycles = 2;
	}
}

/**
 * @brief Tokenize a 16-bit instruction.
 *
 * This tokenizes the 0xCB00-0xCBFF block of instructions.
 *
 * @param instruction Partially created instruction to tokenize.
 */
void bt16_tokenize(BeemuInstruction *instruction)
{
	// now, a very cool fact of life is that we can actually lowkey
	// determine the instruction parameters seperately.
	bt16_determine_type(instruction);
	bt16_determine_params(instruction);
	bt16_determine_clock_cycles(instruction);
}

BeemuInstruction *beemu_tokenizer_tokenize(uint16_t instruction)
{
	BeemuInstruction *inst = calloc(1, sizeof(BeemuInstruction));
	inst->original_machine_code = instruction;
	uint8_t lower_bytes = instruction >> 8;
	if (lower_bytes == 0xCB)
	{
		// Parse 16-bits seperately..
		bt16_tokenize(inst);
	}
	return inst;
};

void beemu_tokenizer_free_token(BeemuInstruction *token)
{
	free(token);
}