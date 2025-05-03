#include "tokenize_load8.h"
#include "tokenize_common.h"
#include <assert.h>
#include <stdbool.h>

BeemuPostLoadOperation tokenize_load8_post_load_op(uint8_t opcode)
{
	switch (opcode) {
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
	// Paralel array to the enum values of BEEMU_TOKENIZER_LOAD8_SUBTYPE
	// if for a specific test in a specific index opcode & _operand == _expetcted_result
	// then it is the load for that specific operand.
	static const struct BeemuTokenizerSubtypeDifferentiator tests[] = {
		// DO NOT USE THE ZEROTH ELEMENT
		{ 0xFF, 0xFF },
		// MAINLINE
		{ 0b11000000, 0b01000000 },
		// D8
		{ 0b11000111, 0b0110 },
		// M16
		{ 0b11000111, 0b00000010 },
		// LDH
		{ 0b11101101, 0b11100000 },
		// ADDR16
		{ 0b11101111, 0b11101010 }
	};

	return instruction_subtype_if_of_instruction_type(
	    opcode,
	    tests,
	    BEEMU_TOKENIZER_LOAD8_INVALID_LOAD,
	    BEEMU_TOKENIZER_LOAD8_ADDR16);
}

/**
 * @brief Parse parameters for the LD 0x40-0x80 main instruction space.
 *
 * These always take two registers, sometimes (HL).
 */
void determine_load8_mainspace_params(BeemuInstruction* instruction, uint8_t opcode)
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
void determine_load8_d8_params(BeemuInstruction* instruction, uint8_t opcode)
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
void determine_load8_m16_params(BeemuInstruction* instruction, uint8_t opcode)
{
	// For 0xn2 we go from r8 -> (r16)
	BeemuParam* register16_param = &(instruction->params.load_params.dest);
	BeemuParam* register8_param = &(instruction->params.load_params.source);
	if ((opcode & 0xF) == 0xA) {
		// But for 0xnA, it is the reverse.
		BeemuParam* temp = register16_param;
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

void determine_load8_ldh_params(BeemuInstruction* instruction, uint8_t opcode)
{
	static const BeemuParam register_a = { .pointer = false,
		.type = BEEMU_PARAM_TYPE_REGISTER_8,
		.value.register_8 = BEEMU_REGISTER_A };
	static const BeemuParam register_c_ptr = { .pointer = true,
		.type = BEEMU_PARAM_TYPE_REGISTER_8,
		.value.register_8 = BEEMU_REGISTER_C };
	// Below is just the opcode and thus meaningless for LDH [C], A and LDH A, [C]
	// but is also harmless to calculate so.
	const uint8_t memory_address_operand = instruction->original_machine_code & 0xFF;
	const BeemuParam memory_address_value = {
		.pointer = true,
		.type = BEEMU_PARAM_TYPE_UINT_8,
		.value.value = memory_address_operand
	};
	// Here combining the 5th LSB and 2nd LSB can be used as an index
	// to calculate the destination and source from an array
	const uint8_t lsb5 = (0x10 & opcode) >> 4;
	const uint8_t lsb2 = (0x02 & opcode) >> 1;
	const uint8_t differentiatior = (lsb5 << 1) | lsb2;
	// And then we can create our indexed array to select the destination
	// AND the source.

	const BeemuParam destination_array[] = {
		memory_address_value,
		register_c_ptr,
		register_a,
		register_a
	};

	const BeemuParam source_array[] = {
		register_a,
		register_a,
		memory_address_value,
		register_c_ptr
	};

	instruction->params.load_params.dest = destination_array[differentiatior];
	instruction->params.load_params.source = source_array[differentiatior];
}

void determine_load8_addr16_params(BeemuInstruction* instruction, uint8_t opcode)
{
	assert(opcode == 0xFA || opcode == 0xEA);
	static const BeemuParam register_a = { .pointer = false,
		.type = BEEMU_PARAM_TYPE_REGISTER_8,
		.value.register_8 = BEEMU_REGISTER_A };
	const uint16_t memory_addr = instruction->original_machine_code & 0xFFFF;
	const BeemuParam ptr = {
		.pointer = true,
		.type = BEEMU_PARAM_TYPE_UINT16,
		.value.value = memory_addr
	};

	if (opcode == 0xEA) {
		instruction->params.load_params.dest = ptr;
		instruction->params.load_params.source = register_a;
	} else {
		// 0xFA case is the reverse
		instruction->params.load_params.dest = register_a;
		instruction->params.load_params.source = ptr;
	}
}

typedef void (*determine_param_function_ptr)(BeemuInstruction*, uint8_t);

// Array used to dispatch to the determine_load8_SUBTYPE_params function
// for a specific BEEMU_TOKENIZER_LOAD8_SUBTYPE, parallel array to the enum
// values
static const determine_param_function_ptr DETERMINE_PARAM_DISPATCH[] = {
	0,
	&determine_load8_mainspace_params,
	&determine_load8_d8_params,
	&determine_load8_m16_params,
	&determine_load8_ldh_params,
	&determine_load8_addr16_params
};

void determine_load8_params(BeemuInstruction* instruction, uint8_t opcode, BEEMU_TOKENIZER_LOAD8_SUBTYPE load_subtype)
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
void determine_load8_clock_cycles(BeemuInstruction* instruction)
{
	instruction->duration_in_clock_cycles = 1;
	if (instruction->params.load_params.source.type == BEEMU_PARAM_TYPE_UINT_8 || instruction->params.load_params.dest.type == BEEMU_PARAM_TYPE_UINT_8) {
		// If the source or destination is a direct 8, then the duration starts as 2.
		instruction->duration_in_clock_cycles++;
	} else if (instruction->params.load_params.source.type == BEEMU_PARAM_TYPE_UINT16 || instruction->params.load_params.dest.type == BEEMU_PARAM_TYPE_UINT16) {
		instruction->duration_in_clock_cycles += 2;
	}
	if (instruction->params.load_params.dest.pointer || instruction->params.load_params.source.pointer) {
		// Memory destructuring due to HL immediately adds a +1 to the clock cycle.
		instruction->duration_in_clock_cycles++;
	}
}

void tokenize_load8(BeemuInstruction* instruction, uint8_t opcode)
{
	BEEMU_TOKENIZER_LOAD8_SUBTYPE load_subtype = load_subtype_if_load(opcode);
	assert(load_subtype != BEEMU_TOKENIZER_LOAD8_INVALID_LOAD);
	instruction->type = BEEMU_INSTRUCTION_TYPE_LOAD_8;
	determine_load8_params(instruction, opcode, load_subtype);
	determine_load8_clock_cycles(instruction);
}
