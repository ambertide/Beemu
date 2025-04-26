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

BEEMU_TOKENIZER_LOAD8_SUBTYPE load_subtype_if_load(uint8_t opcode)
{
	static const uint8_t LOAD_MAINLINE_IDENTIFIER = 0b11000000;
	static const uint8_t LOAD_MAINLINE_IDENTIFIER_EXPECTED = 0b01000000;
	static const uint8_t LOAD_D8_IDENTIFIER = 0b11000111;
	static const uint8_t LOAD_D8_IDENTIFIER_EXPECTED = 0b0110;

	if ((opcode & LOAD_MAINLINE_IDENTIFIER) == LOAD_MAINLINE_IDENTIFIER_EXPECTED)
	{
		return BEEMU_TOKENIZER_LOAD8_MAINLINE;
	}
	else if ((opcode & LOAD_D8_IDENTIFIER) == LOAD_D8_IDENTIFIER_EXPECTED)
	{
		return BEEMU_TOKENIZER_LOAD8_D8;
	}

	return BEEMU_TOKENIZER_LOAD8_INVALID_LOAD;
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

/**
 * @brief Determine the direct 8bit load parameters.
 */
void determine_load8_d8_params(BeemuInstruction *instruction, uint8_t opcode)
{
	const uint8_t destination_differentiatior = (opcode >> 3) & 0b111;
	tokenize_register_param_with_index(&instruction->params.load_params.dest, destination_differentiatior);
	// Source is the operand on the instruction for d8.
	const uint8_t source_direct_value = (instruction->original_machine_code & 0xFF);
	instruction->params.load_params.source.pointer = false;
	instruction->params.load_params.source.type = BEEMU_PARAM_TYPE_UINT_8;
	instruction->params.load_params.source.value.value = source_direct_value;
}

void determine_load8_params(BeemuInstruction *instruction, uint8_t opcode, BEEMU_TOKENIZER_LOAD8_SUBTYPE load_subtype)
{
	instruction->params.load_params.postLoadOperation = tokenize_load8_post_load_op(opcode);
	switch (load_subtype)
	{
	case BEEMU_TOKENIZER_LOAD8_MAINLINE:
		determine_load8_mainspace_params(instruction, opcode);
		break;
	case BEEMU_TOKENIZER_LOAD8_D8:
		determine_load8_d8_params(instruction, opcode);
		break;
	default:
		break;
	}
}

/**
 * @brief Determine the clock cycles taken by a load instruction.
 *
 * @param instruction
 */
void determine_load8_clock_cycles(BeemuInstruction *instruction)
{
	instruction->duration_in_clock_cycles = 1;
	if (instruction->params.load_params.source.type == BEEMU_PARAM_TYPE_UINT_8)
	{
		// If the source is a direct 8, then the duration starts as 2.
		instruction->duration_in_clock_cycles++;
	}
	if (instruction->params.load_params.dest.type == BEEMU_PARAM_TYPE_REGISTER_16 || instruction->params.load_params.source.type == BEEMU_PARAM_TYPE_REGISTER_16)
	{
		// Memory destructuring due to HL immediately adds a +1 to the clock cycle.
		instruction->duration_in_clock_cycles++;
	}
}

void tokenize_load8(BeemuInstruction *instruction, uint8_t opcode)
{
	BEEMU_TOKENIZER_LOAD8_SUBTYPE load_subtype = load_subtype_if_load(opcode);
	assert(load_subtype != BEEMU_TOKENIZER_LOAD8_INVALID_LOAD);
	instruction->type = BEEMU_INSTRUCTION_TYPE_LOAD_8;
	determine_load8_params(instruction, opcode, load_subtype);
	determine_load8_clock_cycles(instruction);
}
