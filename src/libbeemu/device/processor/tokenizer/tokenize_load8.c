#include "tokenize_load8.h"
#include "tokenize_common.h"
#include <stdbool.h>
#include <assert.h>

BeemuPostLoadOperation tokenize_load8_post_load_op(uint8_t opcode)
{
	switch (opcode)
	{
	case 0x22:
		return BEEMU_POST_LOAD_INCREMENT_INDIRECT_DESTINATION;
	case 0x32:
		return BEEMU_POST_LOAD_DECREMENT_INDIRECT_DESTINATION;
	case 0x2A:
		return BEEMU_POST_LOAD_INCREMENT_INDIRECT_SOURCE;
	case 0x3A:
		return BEEMU_POST_LOAD_DECREMENT_INDIRECT_SOURCE;
	default:
		return BEEMU_POST_LOAD_NOP;
	}
}

/**
 * @brief Parse parameters for the LD 0x40-0x80 main instruction space.
 *
 * These always take two registers, sometimes (HL).
 */
void determine_load8_mainspace_params(BeemuInstruction *instruction, uint8_t opcode)
{
	// 4, 5 and 6th most significant bits together decide the destination register
	// since they change once every 8 instructions.
	// Conversely, the first 3 most signifcant bits decide the source register since
	// that pattern RESETS once every 8 instruction.
	const uint8_t destination_differentiator = (opcode >> 3) & 0b111;
	const uint8_t src_differentiator = opcode & 0b111;
	tokenize_register_param_with_index(&instruction->params.load_params.dest, destination_differentiator);
	tokenize_register_param_with_index(&instruction->params.load_params.source, src_differentiator);
}

void determine_load8_params(BeemuInstruction *instruction, uint8_t opcode)
{
	instruction->params.load_params.postLoadOperation = tokenize_load8_post_load_op(opcode);
	// Assuming mainspace for now.
	determine_load8_mainspace_params(instruction, opcode);
}

/**
 * @brief Determine the clock cycles taken by a load instruction.
 *
 * @param instruction
 */
void determine_load8_clock_cycles(BeemuInstruction *instruction)
{
	if (instruction->params.load_params.dest.type == BEEMU_PARAM_TYPE_REGISTER_16 || instruction->params.load_params.source.type == BEEMU_PARAM_TYPE_REGISTER_16)
	{
		instruction->duration_in_clock_cycles = 2;
	}
	else
	{
		instruction->duration_in_clock_cycles = 1;
	}
}

void tokenize_load8(BeemuInstruction *instruction, uint8_t opcode)
{
	instruction->type = BEEMU_INSTRUCTION_TYPE_LOAD_8;
	determine_load8_params(instruction, opcode);
	determine_load8_clock_cycles(instruction);
}
