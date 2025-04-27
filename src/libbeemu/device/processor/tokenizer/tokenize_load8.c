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
	static const uint8_t LOAD_M16_IDENTIFIER = 0b11000111;
	static const uint8_t LOAD_M16_IDENTIFIER_EXPECTED = 0b00000010;
	static const uint8_t LOAD_ADDR8_IDENTIFIER = 0b11101111;
	static const uint8_t LOAD_ADDR8_IDENTIFIER_EXPECTED = 0b11100000;

	// Group them to a struct for easier packing.
	struct BeemuTokenizerLoad8SubtypeDifferentiator
	{
		uint8_t bitwise_and_operand;
		uint8_t bitwise_and_expected_result;
	};

	// Paralel array to the enum values of BEEMU_TOKENIZER_LOAD8_SUBTYPE
	// if for a specific test in a specific index opcode & _operand == _expetcted_result
	// then it is the load for that specific operand.
	static const struct BeemuTokenizerLoad8SubtypeDifferentiator tests[] = {
		// DO NOT USE THE ZEROTH ELEMENT
		{0xFF, 0xFF},
		{LOAD_MAINLINE_IDENTIFIER, LOAD_MAINLINE_IDENTIFIER_EXPECTED},
		{LOAD_D8_IDENTIFIER, LOAD_D8_IDENTIFIER_EXPECTED},
		{LOAD_M16_IDENTIFIER, LOAD_M16_IDENTIFIER_EXPECTED},
		{LOAD_ADDR8_IDENTIFIER, LOAD_ADDR8_IDENTIFIER_EXPECTED}};

	for (int i = 1; i < 5; i++)
	{
		// Test against each possible test and if it returns the expected result
		// we found the relevant subtype.
		struct BeemuTokenizerLoad8SubtypeDifferentiator current_test = tests[i];
		if ((opcode & current_test.bitwise_and_operand) == current_test.bitwise_and_expected_result)
		{
			// This *seems* to be well defined behaviour.
			return i;
		}
	}

	// Otherwise just return the invalid load.
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

/**
 * @brief Determine the 16 bit memory address loads
 */
void determine_load8_m16_params(BeemuInstruction *instruction, uint8_t opcode)
{
	// For 0xn2 we go from r8 -> (r16)
	BeemuParam *register16_param = &(instruction->params.load_params.dest);
	BeemuParam *register8_param = &(instruction->params.load_params.source);
	if ((opcode & 0xF) == 0xA)
	{
		// But for 0xnA, it is the reverse.
		BeemuParam *temp = register16_param;
		register16_param = register8_param;
		register8_param = temp;
	}
	// Anyway, we then need to parse the 16 bit portion.
	tokenize_register16_param_with_index(register16_param, ((opcode >> 4) & 0xF), true, BEEMU_REGISTER_HL);
	// And for the 8 bit portion we get to just put the A register.
	register8_param->pointer = false;
	register8_param->type = BEEMU_PARAM_TYPE_REGISTER_8,
	register8_param->value.register_8 = BEEMU_REGISTER_A;
}

/**
 * @brief Handles calculations for LDH (n80)
 */
void determine_load8_addr8_params(BeemuInstruction *instruction, uint8_t opcode)
{
	BeemuParam register_a = {.pointer = false,
							 .type = BEEMU_PARAM_TYPE_REGISTER_8,
							 .value.register_8 = BEEMU_REGISTER_A};
	uint8_t memory_address_operand = instruction->original_machine_code & 0xFF;
	BeemuParam memory_address_value = {
		.pointer = true,
		.type = BEEMU_PARAM_TYPE_UINT_8,
		.value.value = memory_address_operand};

	if (opcode == 0xE0)
	{
		// For this the addr8 is the dest
		instruction->params.load_params.dest = memory_address_value;
		instruction->params.load_params.source = register_a;
	}
	else
	{
		// For 0xF0 the reverse.
		instruction->params.load_params.source = memory_address_value;
		instruction->params.load_params.dest = register_a;
	}
}

typedef void (*determine_param_function_ptr)(BeemuInstruction *, uint8_t);

// Array used to dispatch to the determine_load8_SUBTYPE_params function
// for a specific BEEMU_TOKENIZER_LOAD8_SUBTYPE, parallel array to the enum
// values
static const determine_param_function_ptr DETERMINE_PARAM_DISPATCH[] = {
	0,
	&determine_load8_mainspace_params,
	&determine_load8_d8_params,
	&determine_load8_m16_params,
	&determine_load8_addr8_params};

void determine_load8_params(BeemuInstruction *instruction, uint8_t opcode, BEEMU_TOKENIZER_LOAD8_SUBTYPE load_subtype)
{
	instruction->params.load_params.postLoadOperation = tokenize_load8_post_load_op(opcode);
	determine_param_function_ptr determine_params_func = DETERMINE_PARAM_DISPATCH[load_subtype];
	determine_params_func(instruction, opcode);
}

/**
 * @brief Determine the clock cycles taken by a load instruction.
 *
 * @param instruction
 */
void determine_load8_clock_cycles(BeemuInstruction *instruction)
{
	instruction->duration_in_clock_cycles = 1;
	if (instruction->params.load_params.source.type == BEEMU_PARAM_TYPE_UINT_8 || instruction->params.load_params.dest.type == BEEMU_PARAM_TYPE_UINT_8)
	{
		// If the source or destination is a direct 8, then the duration starts as 2.
		instruction->duration_in_clock_cycles++;
	}
	if (instruction->params.load_params.dest.pointer || instruction->params.load_params.source.pointer)
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
